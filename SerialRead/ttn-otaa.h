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


#ifdef COMPILE_REGRESSION_TEST
# define FILLMEIN 0
#else
# warning "You must replace the values marked FILLMEIN with real values from the TTN control panel!"
# define FILLMEIN (#dont edit this, edit the lines that use FILLMEIN)
#endif

//lsb format
static const u1_t PROGMEM APPEUI[8]= { 0x11, 0x15, 0x01, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 };
void os_getArtEui (u1_t* buf) { memcpy_P(buf, APPEUI, 8);}

// This should also be in little endian format, see above.
static const u1_t PROGMEM DEVEUI[8]= { 0x8F, 0x4B, 0x3E, 0xDC, 0xDB, 0x2E, 0xA3, 0x00 };
void os_getDevEui (u1_t* buf) { memcpy_P(buf, DEVEUI, 8);}
//lsb
static const u1_t PROGMEM APPKEY[16] = 
{ 0x8B, 0xA4, 0x1A, 0xCC, 0xD5, 0x49, 0x84, 0xC4, 0x2E, 0x5D, 0x49, 0x86, 0xA6, 0x4D, 0x6F, 0x09 };
//{ 0x21, 0xC8, 0x39, 0x66, 0x22, 0xEE, 0xF3, 0xDA, 0xAE, 0x2A, 0x92, 0x89, 0x82, 0x51, 0xD8, 0x29 };
void os_getDevKey (u1_t* buf) {  memcpy_P(buf, APPKEY, 16);}


uint8_t *mydata; //pointer to data to be sent
osjob_t sendjob;

uint8_t DATA_LENGTH=100; //length of data to be sent
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
            LMIC_setDrTxpow(DR_SF7,14);
            LMIC_selectSubBand(1);
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
            FAILED = true;
            break;
    }
}

void do_send(osjob_t* j){
    Serial.println("do_send");
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
    Serial.println(F("Initializing TTN-LoRa settings"));
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
    delete[] mydata;
    mydata = new uint8_t[DATA_LENGTH];
    for(int i = 0;i<DATA_LENGTH;i++){
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
