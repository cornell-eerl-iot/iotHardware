/**
 *  This is a modification of the simple_host example of the Modbus 
 *  for Arduino library from MCCI.
 *  
 *  The purpose of this implementation is to query an array of data
 *  from an external Wattnode Modbus meter via RS-485 using the Modbus
 *  protocol. 
 *  
 *  Comment updated 8/10/2018
 */

#include <Catena.h>
#include "Catena_ModbusRtu.h"

using namespace McciCatena;

Catena gCatena;

/** Initializing the modbus_t telegrams. There is an offset of 1
 * for the addresses we put into the initializer. The addresses we 
 * put into the telegram should be address on the manual minus 1. 
 * REGS (with offset):
 * Reading Regs:
 * Real Power A-C : 1010-1015
 * Power Factor A-C : 1140-1145
 * Reactive Power A-C : 1148-1153
 * Apparant Power A-C : 1156-1161
 * Config Regs:
 * CT-AMPs A : 1603
 * CT-AMPs B : 1604
 * CT-AMPs C : 1605
 * CT Directions : 1606
 *      CT Direction codes:
 *          - 0 : normnal
 *          - 1 : flip A
 * 			- 2 : flip B
 * 			- 3 : flip A and B
 * 			- 4 : flip C
 * 			- 5 : flip A and C
 * 			- 6 : flip B and C
 * 			- 7 : flip all CT's
*/ 

uint16_t writeData[] = {200,200,200}; //Setting CT configurations for device 1
uint16_t writeData2[] = {100,100,100}; //Setting CT configs for device 2
uint16_t writeData3[] = {7}; //Setting CT directions for device 1
uint16_t writeData4[] = {7}; //Setting CT directions for device 2


static const modbus_t T5 = {1,16,1603,3,writeData}; 
static const modbus_t T6 = {2,16,1603,3,writeData2};
static const modbus_t T7 = {1,6,1606,1,writeData3}; 
static const modbus_t T8 = {2,6,1606,1,writeData4}; 


static const modbus_t T1 = {1,3,1010,6,nullptr}; //Reading device 1 real power
static const modbus_t T2 = {1,3,1148,6,nullptr}; //Reading device 1 reactive power
static const modbus_t T3 = {2,3,1010,6,nullptr}; //Reading device 2 real power
static const modbus_t T4 = {2,3,1148,6,nullptr}; //Reading device 2 reactive power

/** Adding telegram to an array. The transmitting of the telegrams will be in order.
 * The telegrams will be queried in a order and loop back. Write functions will only
 * be queried once and then removed.
*/

//static const modbus_t TELEGRAMS[] = {T5,T1,T2}; 
static const modbus_t TELEGRAMS[] = {T5,T6,T7,T8,T1,T2,T3,T4}; 


/**
 *  Modbus object declaration
 *  u8id : node id = 0 for host, = 1..247 for device
 *  u8serno : serial port (use 0 for Serial)
 *  u8txenpin : 0 for RS-232 and USB-FTDI 
 *               or any pin number > 1 for RS-485
 */
cCatenaModbusRtu host(0, A4); // this is host and RS-232 or USB-FTDI
ModbusSerial<decltype(Serial1)> mySerial(&Serial1);

#define kPowerOn        A3

static inline void powerOn(void)
{
        pinMode(kPowerOn, OUTPUT);
        digitalWrite(kPowerOn, HIGH);
}

/**
 * Global Variables
*/
unsigned long u32wait; //wait time between modbus receiving and sending
uint8_t u8state; //State for FSM
bool write_empty = false;

void setup() {
  gCatena.begin();
  powerOn();
  host.begin(&mySerial, 19200); // baud-rate at 19200
  host.setTimeOut( 2000 ); // if there is no answer in 2000 ms, roll over
  host.setTxEnableDelay(100);
  gCatena.registerObject(&host);
  u8state = 0;
    //add telegrams to host.
  for(int i = 0; i < sizeof(TELEGRAMS)/sizeof(TELEGRAMS[0]);i++){
    host.add_telegram(TELEGRAMS[i]);
  }
	u32wait = millis()+100; //100 millisecond delay
	Serial.print("past setup");
}

void loop() {
  switch( u8state ) {
  case 0: 
    if (long(millis() - u32wait) > 0) u8state++; // wait state
    break;
   //polling first set of registers
  case 1: 
    host.setLastError(ERR_SUCCESS);
    write_empty = host.w_telegrams_isEmpty();
    host.query(); // send query (only once)
    u8state++;
    break;
  case 2:
    gCatena.poll(); // check incoming messages
    if (host.getState() == COM_IDLE) {
      u8state=0;
      ERR_LIST lastError = host.getLastError();
      u32wait = (host.getQueryCount() == (host.getTelegramCounter()-1))
       ? millis() + 900 : millis() + 10;

      if (host.getLastError() != ERR_SUCCESS) {
  		  Serial.print("Error ");
  		  Serial.print(int(lastError));
      } else {
        if(write_empty){
          host.i16b_to_float();
          host.print_convertedData();
        }

        //Serial.println("");
      }
      break;
    }
  }
}

