# 1 "c:\\Users\\xiaoy\\Documents\\GitHub\\iotHardware\\Modbus_r-w\\Modbus_r-w.ino"
# 1 "c:\\Users\\xiaoy\\Documents\\GitHub\\iotHardware\\Modbus_r-w\\Modbus_r-w.ino"
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
# 18 "c:\\Users\\xiaoy\\Documents\\GitHub\\iotHardware\\Modbus_r-w\\Modbus_r-w.ino"
# 19 "c:\\Users\\xiaoy\\Documents\\GitHub\\iotHardware\\Modbus_r-w\\Modbus_r-w.ino" 2
# 20 "c:\\Users\\xiaoy\\Documents\\GitHub\\iotHardware\\Modbus_r-w\\Modbus_r-w.ino" 2

using namespace McciCatena;

Catena gCatena;

uint16_t writeData[] = {200,200,200,200};
uint16_t writeData2[] = {100,100,100,100};
uint16_t writeData3[] = {3};


static const modbus_t T5 = {1,16,1602,4,writeData};
static const modbus_t T6 = {2,16,1602,4,writeData2};
static const modbus_t T7 = {1,6,1606,1,writeData3};

static const modbus_t T1 = {1,3,1010,4,nullptr};
static const modbus_t T2 = {1,3,1148,4,nullptr};
static const modbus_t T3 = {2,3,1010,6,nullptr};
static const modbus_t T4 = {2,3,1148,6,nullptr};

static const modbus_t TELEGRAMS[] = {T5,T6,T7,T1,T2,T3,T4};


// data array for modbus network sharing
uint8_t u8state;
/**

 *  Modbus object declaration

 *  u8id : node id = 0 for host, = 1..247 for device

 *  u8serno : serial port (use 0 for Serial)

 *  u8txenpin : 0 for RS-232 and USB-FTDI 

 *               or any pin number > 1 for RS-485

 */
# 51 "c:\\Users\\xiaoy\\Documents\\GitHub\\iotHardware\\Modbus_r-w\\Modbus_r-w.ino"
cCatenaModbusRtu host(0, A4); // this is host and RS-232 or USB-FTDI
ModbusSerial<decltype(Serial1)> mySerial(&Serial1);



static inline void powerOn(void)
{
        pinMode(A3, (0x1));
        digitalWrite(A3, (0x1));
}

/**

 * This is a struct which contains a query to a device

 */
# 65 "c:\\Users\\xiaoy\\Documents\\GitHub\\iotHardware\\Modbus_r-w\\Modbus_r-w.ino"
unsigned long u32wait;


std::vector<float> half_f;

float u16_to_f32(uint16_t h){
    uint16_t sign = (h&0x8000)>>15;
    uint16_t expo = (h&0x7c00)>>10;
    uint16_t manti = h&0x3ff;
    if(expo==0 && manti == 0) return 0.0;
    int e = expo-15;
    float result = pow(2,e);
    result=result*(1+pow(2,-10)*manti);
    result = sign?-result:result;
    return result;
}

void process_data(uint16_t *aray, int array_size){
    for(int i = 0;i<array_size;i++){
        uint16_t sign;
        uint16_t expo;
        uint16_t manti;
        uint32_t f = (aray[++i]<<16)|(aray[i]);
        int f32e = (f&0x7F800000)>>23;
        f32e-=127;

        if(f32e<-24){ // Very small numbers map to zero
            sign=expo=manti=0;
        }else if(f32e<-14){ // Small numbers map to denorms
            sign = ((f>>16)&0x8000);
            expo = 0;
            manti = (0x0400>>(-f32e-14));
        }else if(f32e<=16){ // Normal numbers just lose precision
            sign = ((f>>16)&0x8000);
            expo = ((((f&0x7f800000)-0x38000000)>>13)&0x7c00);
            manti = ((f>>13)&0x03ff);
        }else if(f32e<128){ // Large numbers map to Infinity
            sign = ((f>>16)&0x8000);
            expo = 0x7c00;
            manti = 0;
        }else{ // Infinity and NaN's stay Infinity and NaN's
            sign = ((f>>16)&0x8000);
            expo = 0x7c00;
            manti = ((f>>13)&0x03ff);
        }
        uint16_t h = sign|expo|manti;

        half_f.push_back(u16_to_f32(h));
    }
}

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
       ? millis() + 950 : millis() + 10;

      if (host.getLastError() != ERR_SUCCESS) {
      Serial.print("Error ");
      Serial.print(int(lastError));
      } else {
        if(host.w_telegrams_isEmpty())
          host.i16b_to_float();
          host.print_convertedData();
        }

        //Serial.println("");
      }
      break;
    }


}
