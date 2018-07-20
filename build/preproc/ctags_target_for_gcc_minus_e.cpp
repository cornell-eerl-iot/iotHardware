# 1 "C:\\Users\\xiaoy\\Documents\\GitHub\\iotHardware\\Modbus_Reading\\Modbus_Reading.ino"
# 1 "C:\\Users\\xiaoy\\Documents\\GitHub\\iotHardware\\Modbus_Reading\\Modbus_Reading.ino"
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


# 19 "C:\\Users\\xiaoy\\Documents\\GitHub\\iotHardware\\Modbus_Reading\\Modbus_Reading.ino" 2
# 20 "C:\\Users\\xiaoy\\Documents\\GitHub\\iotHardware\\Modbus_Reading\\Modbus_Reading.ino" 2


using namespace McciCatena;

Catena gCatena;

// data array for modbus network sharing
uint16_t au16data[16];
uint8_t u8state;
uint8_t u8query;

uint16_t reg1 = 1008;
uint16_t reg2 = 1146;
uint16_t numreg= 8;

modbus_t telegram[4];
int telegram_size=4;
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
        pinMode(A3, (0x1));
        digitalWrite(A3, (0x1));
}

void print_regs(float *convertedData, int data_size);
float * i16b_to_float(modbus_t telegram);
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

  telegram[2].u8id = 2; // device address
  telegram[2].u8fct = 3; // function code (this one is registers read)
  telegram[2].u16RegAdd = reg2; // start address in device
  telegram[2].u16CoilsNo = numreg; // number of elements (coils or registers) to read
  telegram[2].au16reg = au16data; // pointer to a memory array in the Arduino

  telegram[3].u8id = 2; // device address
  telegram[3].u8fct = 3; // function code (this one is registers read)
  telegram[3].u16RegAdd = reg2; // start address in device
  telegram[3].u16CoilsNo = numreg; // number of elements (coils or registers) to read
  telegram[3].au16reg = au16data; // pointer to a memory array in the Arduino
}

void loop() {
  for(int j=0;j<telegram_size;j++){
    switch( u8state ) {
      case 0:
        if (long(millis() - u32wait) > 0) u8state++; // wait state
        break;
      //polling first set of registers
      case 1:
        host.setLastError(ERR_SUCCESS);
        host.query( telegram[j] ); // send query (only once)
        if(!(j<telegram_size)){
          Serial.println("");
          }
        u8state++;
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
            float *convertedData = i16b_to_float(telegram[j]);
            print_regs(convertedData,telegram[j].u16CoilsNo/2);
            u32wait = !(j==telegram_size-1) ? (millis()) : (millis()+1000);

          }
          break;
        }
      }
  }

}

/**
 * @brief
 * Converts 16-bit unsigned integer registers to 32 bit floats
 * This funcion will combine two register values together with first value the low bytes and 
 * second values the high bytes. Coil number of telegram should be even.
 * 
 * @param modbus_t telegram 
 *  
 * @return pointer to a float array that contains the converted values
 */
float * i16b_to_float(modbus_t telegram){
  float convertedData[telegram.u16CoilsNo/2];
  uint32_t process32Data[telegram.u16CoilsNo/2];
  uint16_t low;
  uint32_t high;
  uint16_t *au16data = telegram.au16reg;

  for (int i=0; i < telegram.u16CoilsNo; ++i)
  {
    //Serial.print(" ");
    //Serial.print(au16data[i], DEC);
      if(i%2 == 0){
        low = au16data[i];
      }else{
        high = au16data[i];
        high = (high<<16);
        process32Data[(i-1)/2] = low|high;
      }
  }
  memcpy(&convertedData,&process32Data, sizeof(process32Data));
  return convertedData;
}

/**
 * @brief
 * Prints the converted register values to Serial
 * 
 * @param 
 * convertedData: array of data that was processedData
 * data_size: size of the processedData array
 *  
 */
void print_regs(float *convertedData, int data_size){
  for (int i = 0;i<data_size/2;i++){
    Serial.print(convertedData[i],10);
    Serial.print(",");
  }

}
