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


using namespace McciCatena;

Catena gCatena;

uint16_t writeData[] = {200,200,200,200};
uint16_t writeData2[] = {100,100,100,100};


static const modbus_t T5 = {1,16,1602,4,writeData};
static const modbus_t T6 = {2,16,1602,4,writeData2};

static const modbus_t T1 = {1,3,1010,4,nullptr};
static const modbus_t T2 = {1,3,1148,4,nullptr};
static const modbus_t T3 = {2,3,1010,6,nullptr};
static const modbus_t T4 = {2,3,1148,6,nullptr};

static const modbus_t TELEGRAMS[] = {T1,T2,T3,T4,T5,T6}; 


// data array for modbus network sharing
uint8_t u8state; 
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
 * This is a struct which contains a query to a device
 */
unsigned long u32wait;

void setup() {
  gCatena.begin();
  powerOn();
  host.begin(&mySerial, 19200); // baud-rate at 19200
  host.setTimeOut( 2000 ); // if there is no answer in 2000 ms, roll over
  host.setTxEnableDelay(100);
  gCatena.registerObject(&host);
  
  u8state = 0; 
  
  for(int i = 0; i < sizeof(TELEGRAMS)/sizeof(TELEGRAMS[0]);i++){
    host.add_telegram(TELEGRAMS[i]);
  }
	u32wait = millis()+100;
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
    host.query(); // send query (only once)
    u8state++;
    break;
  case 2:
    gCatena.poll(); // check incoming messages
    if (host.getState() == COM_IDLE) {
      u8state=0;
      ERR_LIST lastError = host.getLastError();
      u32wait = (host.getQueryCount() == (host.getTelegramCounter()-1))
       ? millis() + 950 : millis() + 20;

      if (host.getLastError() != ERR_SUCCESS) {
  		  Serial.print("Error ");
  		  Serial.print(int(lastError));
      } else {
        if(host.w_telegrams_isEmpty())
          host.i16b_to_float();
          host.print_convertedData();
        //Serial.println("");
      }
      
    
    break;
  }
}
}

