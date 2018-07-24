
#include <Catena.h>
#include <tuple>
#include "ttn-otaa.h"
#include "Catena_ModbusRtu.h"


#define kPowerOn        A3

using namespace McciCatena;


Catena gCatena;

// data array for modbus network sharing
uint16_t au16data[16];
uint8_t u8state;  

std::tuple<uint16_t,uint16_t> REGBLOCK1(1009,8);


/**
 *  Modbus object declaration
 *  u8id : node id = 0 for host, = 1..247 for device
 *  u8serno : serial port (use 0 for Serial)
 *  u8txenpin : 0 for RS-232 and USB-FTDI 
 *               or any pin number > 1 for RS-485
 */

cCatenaModbusRtu host(0, A4); // this is host and RS-232 or USB-FTDI
ModbusSerial<decltype(Serial1)> mySerial(&Serial1);


static inline void powerOn(void)
{
    pinMode(kPowerOn, OUTPUT);
    digitalWrite(kPowerOn, HIGH);
}

/**
 * This is a struct which contains a query to a device
 */
unsigned long u32wait;

void setup() {
  Serial.println("Starting setup");
  gCatena.begin();
  powerOn();
  host.begin(&mySerial, 19200); // baud-rate at 19200
  host.setTimeOut( 2000 ); // if there is no answer in 2000 ms, roll over
  host.setTxEnableDelay(100);
  gCatena.registerObject(&host);
  host.add_telegram(1,3,std::get<0>(REGBLOCK1)-1 ,std::get<1>(REGBLOCK1),au16data);
  //host.addTelegram(1,3,reg2,numreg,au16data);
  ttn_otaa_init();
  u32wait = millis() + 1000;
  u8state = 0; 
  Serial.println("past setup()");
}


void loop() {
  //Serial.println("In loop");
  switch( u8state ) {
  case 0: 
    //Serial.println("case 0");
    if (long(millis() - u32wait) > 0) u8state++; // wait state
    break;
   //polling first set of registers
  case 1: 
    //Serial.println("case 1");
    host.setLastError(ERR_SUCCESS);
    host.query(); // send query (only once)
    u8state++;
    break;
  case 2:
    //Serial.println("case 2");
    //Gets 1 piece of data through modbus from the electric meter and sending that data to ttn
    gCatena.poll(); // check incoming messages
    if (host.getState() == COM_IDLE) {
      ERR_LIST lastError = host.getLastError();
      if (host.getLastError() != ERR_SUCCESS) {
        Serial.print("Error ");
        Serial.println(int(lastError));
        u8state = 0;
        u32wait = millis()+1000;
      } else {
        //float *convertedData = host.i16b_to_float();
        //host.print_convertedData(); 
        
        mydata = u16_to_u8(au16data,8);  
        do_send(&sendjob);
        Serial.println("Printing data");
        for (int i=0; i<8;i++){
          Serial.print(au16data[i]);Serial.print(" ");
        }Serial.println("");
        Serial.println("sending.."); 
        for(int i = 0; i<16;i++){
          Serial.print(mydata[i],HEX);Serial.print(" ");
        }Serial.println(" ");
        u8state++;
      }
      break;
    }
      case 3:
          if(SEND_COMPLETE){
              u32wait = millis()+1000;
              u8state = 0;
              SEND_COMPLETE = false;
          }else{
            os_runloop_once();
          }
      /*u8state = 0;
      u32wait = millis()+1000;*/
      break;
          
  }
};

