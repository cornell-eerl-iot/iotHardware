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


#define kPowerOn        A3

using namespace McciCatena;

Catena gCatena;

// data array for modbus network sharing
uint16_t au16data[16];
uint32_t process32data[16];
float convertedData[16];
uint8_t u8state; 
uint8_t u8query;
uint16_t reg1 = 1008;
uint16_t reg2 = 1146;
uint16_t numreg= 8;

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
  gCatena.begin();
  powerOn();
  //host.begin(&mySerial, 19200); // baud-rate at 19200
  //host.setTimeOut( 2000 ); // if there is no answer in 2000 ms, roll over
  //host.setTxEnableDelay(100);
  //gCatena.registerObject(&host);
  //numreg = end_reg-start_reg;
  //host.add_telegram(1,3,reg1,numreg,au16data);
  //host.add_telegram(1,3,reg2,numreg,au16data);

  u32wait = millis() + 1000;
  u8state = u8query = 0; 
  
}
void loop() {
  //host.print_telegrams();
  Serial.println("going");
}

