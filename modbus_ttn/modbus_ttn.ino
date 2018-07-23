#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>

// Modbus setup
#include <ModbusRtu.h>
// data array for modbus network sharing
uint16_t au16data[16];
uint32_t process32data[16];
float convertedData[16];
uint8_t u8state;

/**
 *  Modbus object declaration
 *  u8id : node id = 0 for host, = 1..247 for device
 *  u8serno : serial port (use 0 for Serial)
 *  u8txenpin : 0 for RS-232 and USB-FTDI 
 *               or any pin number > 1 for RS-485
 */
Modbus host(0, A4); // this is host and RS-232 or USB-FTDI
ModbusSerial<decltype(Serial1)> mySerial(&Serial1);

#define kPowerOn        A3

static inline void powerOn(void)
{
        pinMode(kPowerOn, OUTPUT);
        digitalWrite(kPowerOn, HIGH);
}

/**
 * This is a struct which contains a query to a device
 */
modbus_t telegram;
unsigned long u32wait;

//TTN setup
#ifdef COMPILE_REGRESSION_TEST
# define FILLMEIN 0
#else
//# error "You must replace the values marked FILLMEIN with real values from the TTN control panel!"
#endif

// This EUI must be in little-endian format, so least-significant-byte
// first. When copying an EUI from ttnctl output, this means to reverse
// the bytes. For TTN issued EUIs the last bytes should be 0xD5, 0xB3,
// 0x70.
static const u1_t PROGMEM APPEUI[8]= { 0x31, 0x27, 0x00, 0xF0, 0x7E, 0xD5, 0xB3, 0x70 };
void os_getArtEui (u1_t* buf) { memcpy_P(buf, APPEUI, 8);}

// This should also be in little endian format, see above.
static const u1_t PROGMEM DEVEUI[8]= { 0x27, 0xB4, 0xC9, 0x75, 0x98, 0x15, 0x50, 0x00 };
void os_getDevEui (u1_t* buf) { memcpy_P(buf, DEVEUI, 8);}

// This key should be in big endian format (or, since it is not really a
// number but a block of memory, endianness does not really apply). In
// practice, a key taken from the TTN console can be copied as-is.
static const u1_t PROGMEM APPKEY[16] = { 0xBB, 0x12, 0x40, 0x5C, 0xFB, 0x75, 0x25, 0xD6, 0x35, 0x1C, 0x3F, 0xF5, 0x37, 0x2F, 0x4E, 0x7F };
void os_getDevKey (u1_t* buf) {  memcpy_P(buf, APPKEY, 16);}

//Data buffer sent to Gateway
uint8_t mydata[] = {0x00,0x00,0x00};
static osjob_t sendjob;

String input;

// Schedule sending every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL = 30;

// Pin mapping
const lmic_pinmap lmic_pins = {
    .nss = 8,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 4,
    .dio = { 3, 6, LMIC_UNUSED_PIN },
};

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

              LMIC_setSeqnoUp(140);
            }
            // Disable link check validation (automatically enabled
            // during join, but not supported by TTN at this time).
            LMIC_setLinkCheckMode(0);
            break;
        case EV_RFU1:
            Serial.println(F("EV_RFU1"));
            break;
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
            os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
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
         default:
            Serial.println(F("Unknown event"));
            break;
    }
}

void do_send(osjob_t* j){
    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("OP_TXRXPEND, not sending"));
    } else {
        // Prepare upstream data transmission at the next possible time
        
        LMIC_setTxData2(1, mydata, sizeof(mydata), 0);
        Serial.println(F("Packet queued"));
    }
    // Next TX is scheduled after TX_COMPLETE event.
}

void setup() {
    delay(5000);
    //while (! Serial)
        ;
    //Modbus master setup    
    powerOn();
    host.begin(&mySerial, 19200); // baud-rate at 19200
    host.setTimeOut( 2000 ); // if there is no answer in 2000 ms, roll over
    host.setTxEnableDelay(100);
    u32wait = millis() + 1000;
    u8state = 0; 
    
    Serial.begin(9600);
    Serial.println(F("Starting"));

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

    // Start job (sending automatically starts OTAA too)
    do_send(&sendjob);
    Serial1.begin(9600);
}

int numreg=8;
bool swap=true;
uint32_t low;
uint32_t high;

void loop() {
    switch( u8state ) {
  case 0: 
    if (long(millis() - u32wait) > 0) {
      swap ? u8state++ : u8state = 3; // wait state
    }
    break;
   //polling first set of registers
  case 1: 
    telegram.u8id = 1; // device address
    telegram.u8fct = 3; // function code (this one is registers read)
    telegram.u16RegAdd = 1008; // start address in device
    telegram.u16CoilsNo = numreg; // number of elements (coils or registers) to read
    telegram.au16reg = au16data; // pointer to a memory array in the Arduino

    host.setLastError(ERR_SUCCESS);
    host.query( telegram ); // send query (only once)
    u8state++;
    break;
  case 2:
    host.poll(); // check incoming messages
    if (host.getState() == COM_IDLE) {
     
      u8state=0;
      ERR_LIST lastError = host.getLastError();
      if (host.getLastError() != ERR_SUCCESS) {
      Serial.print("Error ");
      Serial.print(int(lastError));
      } else {
        Serial.print(millis());
        Serial.print(": Registers: ");
         
          for (int i=0; i < numreg; ++i)
           {
             // Serial.print(" ");
             // Serial.print(au16data[i], BIN);
              if(i%2 == 0){
                low = au16data[i];
              }else{
                high = au16data[i];
                high = (high<<16);
                process32data[(i-1)/2] = low|high; 
              }
            
            }
            memcpy(&convertedData,&process32data, sizeof(process32data));
            for (int i = 0;i<numreg/2;i++){
              Serial.print(" ");
              Serial.print(convertedData[i],DEC);
            }
      }
      //Serial.println("");
      swap = !swap;
      
      u32wait = millis() + 400;
    }
    break;
    
    //polling second set of registers
    case 3: 
    telegram.u8id = 1; // device address
    telegram.u8fct = 3; // function code (this one is registers read)
    telegram.u16RegAdd = 1146; // start address in device
    telegram.u16CoilsNo = numreg; // number of elements (coils or registers) to read
    telegram.au16reg = au16data; // pointer to a memory array in the Arduino

    host.setLastError(ERR_SUCCESS);
    host.query( telegram ); // send query (only once)
    u8state++;
    break;
  case 4:
    host.poll(); // check incoming messages
    if (host.getState() == COM_IDLE) {
      u8state = 0;
      ERR_LIST lastError = host.getLastError();
      if (host.getLastError() != ERR_SUCCESS) {
      Serial.print("Error ");
      Serial.print(int(lastError));
      } else {
        Serial.print(" --- ");
        Serial.print(millis());
        Serial.print(": Registers: ");
        for (int i=0; i < numreg; ++i)
           {
             // Serial.print(" ");
             // Serial.print(au16data[i], BIN);
              if(i%2 == 0){
                low = au16data[i];
              }else{
                high = au16data[i];
                high = (high<<16);
                process32data[(i-1)/2] = low|high; 
              }
            
            }
            memcpy(&convertedData,&process32data, sizeof(process32data));
            for (int i = 0;i<numreg/2;i++){
              Serial.print(" ");
              Serial.print(convertedData[i],DEC);
            }
      }
      Serial.println("");
      swap = !swap;
      
      u32wait = millis() + 400;
    }
    break;
    os_runloop_once();
    while(Serial1.available()){
          char s = Serial1.read();

          //end of Serial/UART transmission from Sensor Board
          if (s=='!'){
            input.getBytes(mydata,input.length()+1);
            Serial.println(input);
            input = "";
          }
          //not the end of transmission
          else{
            input += s;
          }
        }
    }}


