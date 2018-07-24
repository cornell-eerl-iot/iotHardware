/**
 *  This is a modification of the simple_host example of the Modbus 
 *  for Arduino library from MCCI.
 *  
 *  The purpose of this implementation is to query an array of data
 *  from an external Wattnode Modbus meter via RS-485. 
 *  
 *  The registers to query are:  
 *  Real Power: 
 *      Sum: 1009-10
 *      A,B,C: 1011-2, 1013-4, 1015-6
 *  Reactive Power:
 *      Sum: 1147-8
 *      A,B,C: 1149-50, 1151-2, 1153-4
 */


#include <Catena.h>
#include "Catena_ModbusRtu.h"
#include <tuple>

#define kPowerOn        A3

using namespace McciCatena;


Catena gCatena;

// data array for modbus network sharing
uint16_t au16data[16];
uint8_t u8state;  

std::tuple<uint16_t,uint16_t> REGBLOCK1(1701,8);


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
  Serial.println("Starting setup()");
  gCatena.begin();
  powerOn();
  host.begin(&mySerial, 19200); // baud-rate at 19200
  host.setTimeOut( 2000 ); // if there is no answer in 2000 ms, roll over
  host.setTxEnableDelay(100);
  gCatena.registerObject(&host);
  host.add_telegram(1,3,std::get<0>(REGBLOCK1)-1 ,std::get<1>(REGBLOCK1),au16data);
  //host.addTelegram(1,3,reg2,numreg,au16data);

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
    gCatena.poll(); // check incoming messages
    if (host.getState() == COM_IDLE) {
      u8state=0;
      ERR_LIST lastError = host.getLastError();
      if (host.getLastError() != ERR_SUCCESS) {
  		  Serial.print("Error ");
  		  Serial.print(int(lastError));
      } else {
        Serial.println("Printing data");
        for (int i=0; i<8;i++){
          Serial.print(au16data[i]);Serial.print(" ");
        }Serial.println("");
        
        //float *convertedData = host.i16b_to_float();
        //host.print_convertedData();
        u32wait = millis()+1000;
      }
      break;
    }
  }
};

