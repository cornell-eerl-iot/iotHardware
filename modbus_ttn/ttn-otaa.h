/**
 * Modified version of the ttn-otaa.ino file found in arduino-lmic
 * library to be compatible with modbus_ttn.ino. 
 * This version is a header with additional functions that 
 * reduces the size of the payload to be send. Also supports reset
 * 
 * Comment Updated 8/10/2018
*/


#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include "scheduler.h"


#ifdef COMPILE_REGRESSION_TEST
# define FILLMEIN 0
#else
# warning "You must replace the values marked FILLMEIN with real values from the TTN control panel!"
# define FILLMEIN (#dont edit this, edit the lines that use FILLMEIN)
#endif

static const u1_t PROGMEM APPEUI[8]= {  0x20, 0xFB, 0x00, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 };//0xEC, 0xEE, 0x00, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 };
void os_getArtEui (u1_t* buf) { memcpy_P(buf, APPEUI, 8);}

// This should also be in little endian format, see above.
static const u1_t PROGMEM DEVEUI[8]= { 0x93, 0x2B, 0xF5, 0x93, 0x9B, 0xAF, 0x60, 0x00};//0x53, 0x01, 0x00, 0x00, 0x01, 0xCC, 0x02, 0x00 };
void os_getDevEui (u1_t* buf) { memcpy_P(buf, DEVEUI, 8);}

static const u1_t PROGMEM APPKEY[16] = { 0x21, 0xC8, 0x39, 0x66, 0x22, 0xEE, 0xF3, 0xDA, 0xAE, 0x2A, 0x92, 0x89, 0x82, 0x51, 0xD8, 0x29};//0xA2, 0x0E, 0x07, 0x34, 0x6E, 0x98, 0x71, 0xE0, 0x6C, 0x71, 0x98, 0x62, 0x53, 0x1A, 0xD4, 0xA3 };
void os_getDevKey (u1_t* buf) {  memcpy_P(buf, APPKEY, 16);}


uint8_t *mydata; //pointer to data to be sent
osjob_t sendjob;

uint8_t DATA_LENGTH=1; //length of data to be sent
bool SEND_COMPLETE = true; //indicator used to tell us when sending data is done.
bool JOINED = false; //inducator when we joined the network
bool FAILED = false; //connection lost
uint8_t Connection_Num = 0; //number of reconnects 


// Pin mapping
#if defined(ARDUINO_SAMD_FEATHER_M0)
// Pin mapping for Adafruit Feather M0 LoRa, etc.
const lmic_pinmap lmic_pins = {
    .nss = 8,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 4,
    .dio = {3, 6, LMIC_UNUSED_PIN},
    .rxtx_rx_active = 0,
    .rssi_cal = 8,              // LBT cal for the Adafruit Feather M0 LoRa, in dB
    .spi_freq = 8000000,
};
#elif defined(ARDUINO_CATENA_4551)
// Pin mapping for Murata module / Catena 4551
const lmic_pinmap lmic_pins = {
        .nss = 7,
        .rxtx = 29,
        .rst = 8,
        .dio = { 25,    // DIO0 (IRQ) is D25
                 26,    // DIO1 is D26
                 27,    // DIO2 is D27
               },
        .rxtx_rx_active = 1,
        .rssi_cal = 10,
        .spi_freq = 8000000     // 8MHz
};
#else
# error "Unknown target"
#endif


void onEvent (ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(": ");
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            Serial.println(F("EV_SCAN_TIMEOUT"));
            break;
        case EV_BEACON_FOUND:
            Serial.println(F("EV_BEACON_FOUND"));
            break;
        case EV_BEACON_MISSED:
            Serial.println(F("EV_BEACON_MISSED"));
            break;
        case EV_BEACON_TRACKED:
            Serial.println(F("EV_BEACON_TRACKED"));
            break;
        case EV_JOINING:
            Serial.println(F("EV_JOINING"));
            break;
        case EV_JOINED:
            Serial.println(F("EV_JOINED"));
            {

              u4_t netid = 0;
              devaddr_t devaddr = 0;
              u1_t nwkKey[16];
              u1_t artKey[16];
              LMIC_getSessionKeys(&netid, &devaddr, nwkKey, artKey);
              Serial.print("netid: ");
              Serial.println(netid, DEC);
              Serial.print("devaddr: ");
              Serial.println(devaddr, HEX);
              Serial.print("artKey: ");
              for (int i=0; i<sizeof(artKey); ++i) {
                if (i != 0)
                  Serial.print("-");
                Serial.print(artKey[i], HEX);
              }
              Serial.println("");
              Serial.print("nwkKey: ");
              for (int i=0; i<sizeof(nwkKey); ++i) {
                      if (i != 0)
                              Serial.print("-");
                      Serial.print(nwkKey[i], HEX);
              }
              Serial.println("");
              JOINED = true;
            }
            
            // Disable link check validation (automatically enabled
            // during join, but because slow data rates change max TX
      // size, we don't use it in this example.
            LMIC_setLinkCheckMode(0);
            break;
        /*
        || This event is defined but not used in the code. No
        || point in wasting codespace on it.
        ||
        || case EV_RFU1:
        ||     Serial.println(F("EV_RFU1"));
        ||     break;
        */
        case EV_JOIN_FAILED:
            Serial.println(F("EV_JOIN_FAILED"));
            FAILED = true;
            break;
        case EV_REJOIN_FAILED:
            Serial.println(F("EV_REJOIN_FAILED"));
            FAILED = true;
            break;
            break;
        case EV_TXCOMPLETE:
            Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
            if (LMIC.txrxFlags & TXRX_ACK)
              Serial.println(F("Received ack"));
            if (LMIC.dataLen) {
              Serial.println(F("Received "));
              Serial.println(LMIC.dataLen);
              Serial.println(F(" bytes of payload"));
            }
            // Schedule next transmission
            SEND_COMPLETE = true;
            break;
        case EV_LOST_TSYNC:
            Serial.println(F("EV_LOST_TSYNC"));
            FAILED = true;
            break;
        case EV_RESET:
            Serial.println(F("EV_RESET"));
            break;
        case EV_RXCOMPLETE:
            // data received in ping slot
            Serial.println(F("EV_RXCOMPLETE"));
            break;
        case EV_LINK_DEAD:
            Serial.println(F("EV_LINK_DEAD"));
            FAILED = true;
            break;
        case EV_LINK_ALIVE:
            Serial.println(F("EV_LINK_ALIVE"));
            break;
        /*
        || This event is defined but not used in the code. No
        || point in wasting codespace on it.
        ||
        || case EV_SCAN_FOUND:
        ||    Serial.println(F("EV_SCAN_FOUND"));
        ||    break;
        */
        case EV_TXSTART:
            Serial.println(F("EV_TXSTART"));
            break;
        default:
            Serial.print(F("Unknown event: "));
            Serial.println((unsigned) ev);
            FAILED - true;
            break;
    }
}

void do_send(osjob_t* j){
    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("OP_TXRXPEND, not sending"));
    } else {
        // Prepare upstream data transmission at the next possible time.
        LMIC_setTxData2(1, mydata, DATA_LENGTH, 0);
        Serial.println(F("Packet queued"));
    }
    SEND_COMPLETE = false;
}

void ttn_otaa_init(){
    //delay(5000);
    //while (! Serial);
    //Serial.println(F("Initializing TTN-LoRa settings"));
    #ifdef VCC_ENABLE
    // For Pinoccio Scout boards
    pinMode(VCC_ENABLE, OUTPUT);
    digitalWrite(VCC_ENABLE, HIGH);
    //delay(1000);
    #endif
    // LMIC init
    os_init();
    // Reset the MAC state. Session and pending data transfers will be discarded.
    LMIC_reset();

    LMIC_setLinkCheckMode(0);
    LMIC_setDrTxpow(DR_SF7,14);
    LMIC_selectSubBand(1);
    mydata = new uint8_t[1];
    for(int i = 0;i<1;i++){
        mydata[i]=0xFF;
    }
    // Start job (sending automatically starts OTAA too)
    
    do_send(&sendjob); //establish connection
    Connection_Num++;
    JOINED = false;
    //Wait until we are able to join the network before start polling.
    while(!JOINED){// Just want to join, will send in FSM.
        os_runloop_once();   
    }
    FAILED = false;
}

/**
 * @brief
 * Converts 16-bit unsigned integer data representations from the modbus output
 * to 8-bit unsigned representations. The 16-bit array from modbus should 
 * have lower 2 bytes followed by the higher 2 bytes. The 8-bit output 
 * from this function will be high bytes to lower bytes. We assume that full 
 * registers are 32-bits or 4 bytes.
 * 
 * @param 16-bit array and its array length
 * @return the processed 8-bit array
 */
uint8_t *u16_to_u8(uint16_t *aray, int array_size){
    DATA_LENGTH = array_size*2;
  uint8_t *out = new uint8_t[DATA_LENGTH];
  uint8_t lowByte, highByte;
  for(int i=0; i<array_size; i++){
      lowByte = aray[i];
      highByte = aray[i]>>8;
      out[i*2] = highByte;
      out[i*2+1] = lowByte;
  }
  return out;
}

/**
 * @brief
 * Converts 32-bit single precision floating point representation to 16-bit 
 * half precision floating point representation in order to reduce space used. 
 * 4bytes->2bytes
 * 
 * @param 32-bit array containing IEEE single precision floating point reps 
 * and its array length
 * @return the processed 16-bit array containing IEEE half precision floating 
 * point reps
 */
uint16_t *f32_to_f16(uint16_t *aray, int array_size){
    uint16_t *temp = new uint16_t[array_size/2];
    for(int i = 0;i<array_size;i++){
        uint32_t f = (aray[i])|(aray[++i]<<16);
        uint16_t h = ((f>>16)&0x8000)|((((f&0x7f800000)-0x38000000)>>13)&0x7c00)|((f>>13)&0x03ff);
        temp[(i-1)/2] = h;
    }
    return temp;
}

/**
 * @brief
 * Processes the data converting from 16-bit regs single precision float repr 
 * to 8bit regs half precision repr. Half precision floats loses precision and 
 * have a lower range. Soft limit is now at 65536.0. However, this is up to
 * the converter and the user to decide when the cutoff will be. Hard limit is 
 * at around 200000 where the value will wrap.
 * As the numbers get larger, precision will decrease such as lack of decimals. 
 * 
 * @param 16-bit array containing IEEE single precision floating point reps in 
 * lower byte followed by higher byte format and its array length.
 * Queue type that will be added to the existing queue.
 * 
 */
void process_data(uint16_t *aray, int array_size, queue_t* queue){
    for(int i = 0;i<array_size;i++){
        uint16_t sign; 
        uint16_t expo;
        uint16_t manti;
        uint32_t f = (aray[++i]<<16)|(aray[i]);
        int f32e = (f&0x7F800000)>>23;
        f32e-=127;
        
        if(f32e<-24){ // Very small numbers map to zero
            sign=expo=manti=0;
        }else if(f32e<-14){ // Small numbers map to denorms
            sign = ((f>>16)&0x8000);
            expo = 0;
            manti = (0x0400>>(-f32e-14));
        }else if(f32e<=16){ // Normal numbers just lose precision
            sign = ((f>>16)&0x8000);
            expo = ((((f&0x7f800000)-0x38000000)>>13)&0x7c00);
            manti = ((f>>13)&0x03ff);
        }else if(f32e<128){ // Large numbers map to Infinity
            sign = ((f>>16)&0x8000);
            expo = 0x7c00;
            manti = 0;
        }else{ // Infinity and NaN's stay Infinity and NaN's
            sign = ((f>>16)&0x8000);
            expo = 0x7c00;
            manti = ((f>>13)&0x03ff);
        }
        uint16_t h = sign|expo|manti;
        queue->buffer.push_back(h&0x00FF); //low byte, high byte
        queue->buffer.push_back((h&0xFF00)>>8);  
        
        //float result = u16_to_f32(h);
    }
}

/**
 * @brief
 * Takes in 16-bit half precision float representation and converts it to a 
 * float type number.
 * There are multiple ways to do this. This is the formula version. There 
 * are versions using memcpy and union. The differences are not that 
 * significant.
 * 
 * @param 16-bit representation
 * 
 */
float u16_to_f32(uint16_t h){
    uint16_t sign = (h&0x8000)>>15;
    uint16_t expo = (h&0x7c00)>>10;
    uint16_t manti = h&0x3ff;
    if(expo==0 && manti == 0) return 0.0;
    int e = expo-15;
    float result = pow(2,e);
    result=result*(1+pow(2,-10)*manti);
    result = sign?-result:result;
    return result;
}




