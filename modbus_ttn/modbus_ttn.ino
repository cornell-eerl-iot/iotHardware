
#include <Catena.h>
#include <tuple>
#include "ttn-otaa.h"
#include "Catena_ModbusRtu.h"
#include <RTCZero.h>
#include <algorithm>

#define kPowerOn        A3

using namespace McciCatena;


Catena gCatena;
RTCZero rtc;
// data array for modbus network sharing

std::tuple<uint16_t,uint16_t> REGBLOCK1(1701,6);
std::tuple<uint16_t,uint16_t> REGBLOCK2(1707,6);
std::tuple<uint16_t,uint16_t> REGBLOCK3(1601,4);
std::tuple<uint16_t,uint16_t> REGBLOCK4(1605,4);

uint8_t buffer_time = 5;
uint8_t sample_size = 20; //in bytes
volatile uint8_t t=1; //time counter for RTC interrupts
uint8_t SAMPLE_PERIOD = 5;
uint8_t RATE = 1;

/**
 *  Modbus object declaration
 *  u8id : node id = 0 for host, = 1..247 for device
 *  u8serno : serial port (use 0 for Serial)
 *  u8txenpin : 0 for RS-232 and USB-FTDI 
 *               or any pin number > 1 for RS-485
 */

cCatenaModbusRtu host(0, A4); // this is host and RS-232 or USB-FTDI
ModbusSerial<decltype(Serial1)> mySerial(&Serial1);
volatile uint8_t u8state = 0;
queue_t *new_tail;
volatile uint8_t querying_count;
volatile uint8_t accumulate_count;
volatile int queue_count;

static inline void powerOn(void)
{
    pinMode(kPowerOn, OUTPUT);
    digitalWrite(kPowerOn, HIGH);
}


volatile unsigned long u32wait;

void setup() {
  Serial.println("Starting setup");
  gCatena.begin();
  powerOn();
  host.begin(&mySerial, 19200); // baud-rate at 19200
  host.setTimeOut( 2000 ); // if there is no answer in 2000 ms, roll over
  host.setTxEnableDelay(100);
  gCatena.registerObject(&host);
  host.add_telegram(1,3,std::get<0>(REGBLOCK1)-1 ,std::get<1>(REGBLOCK1));
  host.add_telegram(1,3,std::get<0>(REGBLOCK2)-1 ,std::get<1>(REGBLOCK2));
  host.add_telegram(1,3,std::get<0>(REGBLOCK3)-1 ,std::get<1>(REGBLOCK3));
  host.add_telegram(1,3,std::get<0>(REGBLOCK4)-1 ,std::get<1>(REGBLOCK4));

  ttn_otaa_init();

  //Initalize RTC interrupts and timers.
  
  do_send(&sendjob); //establish connection
  while(!JOINED){//||!SEND_COMPLETE){
    os_runloop_once();   
  }
  rtc.begin();
  t=5;
  rtc.setAlarmSeconds(t);             
  rtc.attachInterrupt(alarmMatch);
  rtc.setSeconds(0);
  rtc.enableAlarm(rtc.MATCH_SS);

  u8state = 6;
  u32wait = millis() + 1000;
  querying_count = host.getTelegramSize();
  accumulate_count = 1;
  Serial.println("past setup()");
}

//have alarm be a setting where the alarm will trigger that it is time to sample again.

void loop() {
 
    switch(u8state){ 
          case 0:
            new_tail = new queue_t;
            accumulate_count = SAMPLE_PERIOD;
            u8state++;
            break;

          case 1: 
            if (long(millis() - u32wait) > 0) u8state++; // wait state
            
            break;
            
          case 2:
            host.setLastError(ERR_SUCCESS);
            host.query(); // send query (only once)
            u8state++;
            querying_count --;
            break;
          
          case 3:
            gCatena.poll(); // check incoming messages
            if (host.getState() == COM_IDLE) {
              ERR_LIST lastError = host.getLastError();
              if (host.getLastError() != ERR_SUCCESS) {
                Serial.print("Error ");
                Serial.println(int(lastError));
              } else {
                process_data(host.getContainer(),host.getContainerCurrSize(),new_tail);  
                /*for(int i = 0; i<new_tail->buffer.size();i++){
                  Serial.print(new_tail->buffer[i],HEX);Serial.print(" ");
                }Serial.println(" "); */              
              }
              u8state = (querying_count==0) ? u8state+1 : 1;
              u32wait = millis()+10;
            }
            break;
          case 4:
            //Serial.println("push tail");
            //if(accumulate_count == 0) push_tail(new_tail);
            u8state++;
          break;
          case 5:
            if(SEND_COMPLETE && queue){
              queue_t *head = pop_front_queue();
              queue_count--;
              DATA_LENGTH = head->buffer.size();
              mydata = new uint8_t[DATA_LENGTH];
              std::copy ( head->buffer.begin(), head->buffer.end(), mydata );
              do_send(&sendjob);

              Serial.print("sending.."); Serial.println(DATA_LENGTH);
              Serial.print("queue count: ");Serial.println(queue_count);
              /*for(int i = 0; i<head->buffer.size();i++){
                Serial.print(mydata[i],HEX);Serial.print(" ");
              }Serial.println(" ");*/
            }
            u8state++;
            break;
          
          case 6:
            os_runloop_once();   
          break;
    }
};

/**
 * @brief
 * Interrupt handler for alarm ring
 * 
 */
void alarmMatch()
{
  accumulate_count--;
  if(accumulate_count == 0){
    Serial.println("pushing tail");
    queue_count++;
    push_tail(new_tail);
    u8state = 0;
  }else{
    u8state = 1;
  }
  u32wait = millis()+10;
  querying_count = host.getTelegramSize();
  t+=RATE;
  if(t>=60) t-=60;
  rtc.setAlarmSeconds(t);

}
