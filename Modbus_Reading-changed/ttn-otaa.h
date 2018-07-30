
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>


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


uint8_t *mydata;
osjob_t sendjob;

u1_t DATA_LENGTH;
bool SEND_COMPLETE = false; //indicator used to tell us when sending data is done.


const unsigned TX_INTERVAL = 5;

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
            break;
        case EV_REJOIN_FAILED:
            Serial.println(F("EV_REJOIN_FAILED"));
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
    // Next TX is scheduled after TX_COMPLETE event.
}

void ttn_otaa_init(){
    delay(5000);
    while (! Serial)
        ;
    Serial.println(F("Initializing TTN-LoRa settings"));
    #ifdef VCC_ENABLE
    // For Pinoccio Scout boards
    pinMode(VCC_ENABLE, OUTPUT);
    digitalWrite(VCC_ENABLE, HIGH);
    delay(1000);
    #endif
    // LMIC init
    os_init();
    // Reset the MAC state. Session and pending data transfers will be discarded.
    LMIC_reset();

    LMIC_setLinkCheckMode(0);
    LMIC_setDrTxpow(DR_SF7,14);
    LMIC_selectSubBand(1);
    SEND_COMPLETE = false;
    //mydata = new uint8_t[4];
    // Start job (sending automatically starts OTAA too)
    //do_send(&sendjob);
}

/**
 * @brief
 * Converts 16-bit unsigned integer data representations from the modbus output
 * to 8-bit unsigned representations. The 16-bit array from modbus should have lower 2 bytes
 * followed by the higher 2 bytes. The 8-bit output from this function will be high bytes to 
 * lower bytes. We assume that full registers are 32-bits or 4 bytes.
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
 * Converts 32-bit single precision floating point numbers to 16-bit half precision floating
 * point numbers.
 * 
 * @param float represented as uint32_t
 * @return uint16_t representation of float 
// bit 15 is the sign
// bits 14..11 are the exponent
// bits 10..0 are the fraction
 */
uint16_t f32_to_f16(uint32_t f){
    uint8_t sign = (f&0x80000000)>>32;
    uint8_t exponent = (f&0x7F800000)>>23;
    uint32_t manti =  (f&0x00FFFFFFFF);
    uint16_t output;
    bound = exponent - 127;
    if(bound <-24){
        output = 0;
    }else if (bound<-14){
        output = 
    }
}



#include <iostream>
#include <cmath>

using namespace std;


int main()
{
    bool flag = false;
    uint32_t f = 0x3b380000; 
    uint16_t h = ((f>>16)&0x8000)|((((f&0x7f800000)-0x38000000)>>13)&0x7c00)|((f>>13)&0x03ff);
    if((((f&0x7f800000)-0x38000000 )>>23 &0xf )== 0 or ((f&0x7f800000)-0x38000000 )>>23 &0xf==0x0f)
    {
        flag = true;
    }
    if(flag) cout<<"yes"<<endl;
    
    int e =(((f&0x7f800000)-0x38000000 )>>23 &0x1f );
    cout<<e<<endl;
    e -= 15;
    int man = (f>>13)&0x03ff;
    float result = pow(2,e);
    result=result*(1+pow(2,-10)*man);
    
    cout<<e<<endl;
    
    cout<<result<<endl;
}


    uint16_t aray[] = {0x0000,0x434a,0x0000,0x45cb};
    int array_size = 4;
    
    uint8_t *temp = new uint8_t[array_size];
    uint8_t lowByte, highByte;
    uint32_t f;
    for(int i = 0;i<array_size;i++){
        f = (aray[i])|(aray[i+1]<<16);
        uint16_t h = ((f>>16)&0x8000)|((((f&0x7f800000)-0x38000000)>>13)&0x7c00)|((f>>13)&0x03ff);
        lowByte = h&0x00FF;
        highByte = (h&0xFF00)>>8;
        temp[i] = lowByte;
        temp[i++] = highByte;
    }

    for(int i = 0; i<array_size;i++){
        printf("%d ",temp[i]);
    }printf("\n");
    int e =(((f&0x7f800000)-0x38000000 )>>23 &0x1f );
    e -= 15;
    int man = (f>>13)&0x03ff;
    float result = pow(2,e);
    result=result*(1+pow(2,-10)*man);
    
    cout<<result<<endl;
    
    return 0;
}