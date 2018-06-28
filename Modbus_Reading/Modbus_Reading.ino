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

#include <ModbusRtu.h>

// data array for modbus network sharing
uint16_t au16data[16];
uint32_t process32data[16];
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

void setup() {
  powerOn();
  host.begin(&mySerial, 19200); // baud-rate at 19200
  host.setTimeOut( 2000 ); // if there is no answer in 2000 ms, roll over
  host.setTxEnableDelay(100);
  u32wait = millis() + 1000;
  u8state = 0; 
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
            for (int i = 0;i<numreg/2;i++){
              Serial.print(" ");
              Serial.print(process32data[i],DEC);
            }
      }
      //Serial.println("");
		  swap = !swap;
      u32wait = millis() + 1000;
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
            for (int i = 0;i<numreg/2;i++){
              Serial.print(" ");
              Serial.print(process32data[i],DEC);
            }
      }
		  Serial.println("");
		  swap = !swap;
      u32wait = millis() + 1000;
    }
    break;
  }
}

