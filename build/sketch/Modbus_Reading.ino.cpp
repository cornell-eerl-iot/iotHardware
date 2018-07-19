#include <Arduino.h>
#line 1 "c:\\Users\\xiaoy\\Documents\\GitHub\\iotHardware\\Modbus_Reading\\Modbus_Reading.ino"
#line 1 "c:\\Users\\xiaoy\\Documents\\GitHub\\iotHardware\\Modbus_Reading\\Modbus_Reading.ino"
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

#define kPowerOn        A3

#line 48 "c:\\Users\\xiaoy\\Documents\\GitHub\\iotHardware\\Modbus_Reading\\Modbus_Reading.ino"
static void powerOn(void);
#line 61 "c:\\Users\\xiaoy\\Documents\\GitHub\\iotHardware\\Modbus_Reading\\Modbus_Reading.ino"
void setup();
#line 85 "c:\\Users\\xiaoy\\Documents\\GitHub\\iotHardware\\Modbus_Reading\\Modbus_Reading.ino"
void loop();
#line 48 "c:\\Users\\xiaoy\\Documents\\GitHub\\iotHardware\\Modbus_Reading\\Modbus_Reading.ino"
static inline void powerOn(void)
{
        pinMode(kPowerOn, OUTPUT);
        digitalWrite(kPowerOn, HIGH);
}

/**
 * This is a struct which contains a query to a device
 */
modbus_t telegram[2];

unsigned long u32wait;

void setup() {
  gCatena.begin();
  powerOn();
  host.begin(&mySerial, 19200); // baud-rate at 19200
  host.setTimeOut( 2000 ); // if there is no answer in 2000 ms, roll over
  host.setTxEnableDelay(100);
  gCatena.registerObject(&host);
  //numreg = end_reg-start_reg;
  u32wait = millis() + 1000;
  u8state = u8query = 0; 
  
  telegram[0].u8id = 1; // device address
  telegram[0].u8fct = 3; // function code (this one is registers read)
  telegram[0].u16RegAdd = reg1; // start address in device
  telegram[0].u16CoilsNo = numreg; // number of elements (coils or registers) to read
  telegram[0].au16reg = au16data; // pointer to a memory array in the Arduino

  telegram[1].u8id = 1; // device address
  telegram[1].u8fct = 3; // function code (this one is registers read)
  telegram[1].u16RegAdd = reg2; // start address in device
  telegram[1].u16CoilsNo = numreg; // number of elements (coils or registers) to read
  telegram[1].au16reg = au16data; // pointer to a memory array in the Arduino
}

void loop() {
  uint32_t low;
  uint32_t high;
  switch( u8state ) {
  case 0: 
    if (long(millis() - u32wait) > 0) u8state++; // wait state
    break;
   //polling first set of registers
  case 1: 
    host.setLastError(ERR_SUCCESS);
    host.query( telegram[u8query] ); // send query (only once)
    if(u8query==0){
      Serial.println("");
      //Serial.print(millis());
      //Serial.print(": Registers:");   
      }
    u8state++;
    u8query++;
    if (u8query > 1) u8query = 0;
    break;
  case 2:
    gCatena.poll(); // check incoming messages
    if (host.getState() == COM_IDLE) {
      
      u8state=0;
      ERR_LIST lastError = host.getLastError();
      
      if (host.getLastError() != ERR_SUCCESS) {
  		  Serial.print("Error ");
  		  Serial.print(int(lastError));
      } else {
        
        for (int i=0; i < numreg; ++i)
        {
        //Serial.print(" ");
        //Serial.print(au16data[i], DEC);
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
          Serial.print(convertedData[i],DEC);
          if(i+1!=numreg/2)
            Serial.print(",");
        }
        if(u8query-1==0)
         Serial.print(",");
        u32wait = ((u8query-1)==0) ? (millis() + 1) : (millis()+1000);
      }
      break;
    }
  }
}


