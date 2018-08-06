
#include <Catena.h>
#include "ttn-otaa.h"
#include "Catena_ModbusRtu.h"
#include <RTCZero.h>
#include <algorithm>
#include <utility>

#define kPowerOn        A3

using namespace McciCatena;


RTCZero rtc;
// data array for modbus network sharing

//User set variables
static const modbus_t T1 = {1,3,1010,4,nullptr};
static const modbus_t T2 = {1,3,1148,4,nullptr};
static const modbus_t T3 = {2,3,1010,6,nullptr};
static const modbus_t T4 = {2,3,1148,6,nullptr};

static const modbus_t TELEGRAMS[] = {T1,T2,T3,T4}; 

uint8_t SAMPLE_PERIOD = 5; //Number of samples to collect before sending over LoRa.
uint8_t SAMPLE_RATE = 1; //Time in seconds between samples from WattNode [1:255]


/**
 *  Modbus object declaration
 *  u8id : node id = 0 for host, = 1..247 for device
 *  u8serno : serial port (use 0 for Serial)
 *  u8txenpin : 0 for RS-232 and USB-FTDI 
 *               or any pin number > 1 for RS-485
 */

cCatenaModbusRtu host(0, A4); // this is host and RS-232 or USB-FTDI
ModbusSerial<decltype(Serial1)> mySerial(&Serial1);

//Global Variables - user do not need to define.
volatile uint8_t u8state = 0;
queue_t *new_tail = new queue_t;
volatile uint8_t querying_count=0;
volatile uint8_t accumulate_count=0;
volatile uint8_t queue_count=0;
void connectionReset();
uint8_t sample_rate = SAMPLE_RATE-1;

static inline void powerOn(void)
{
    pinMode(kPowerOn, OUTPUT);
    digitalWrite(kPowerOn, HIGH);
}


volatile unsigned long u32wait = 0;

void setup() {
  Serial.println("Starting setup");
  powerOn();  
  host.begin(&mySerial, 19200); // baud-rate at 19200
  host.setTimeOut( 2000 ); // if there is no answer in 2000 ms, roll over
  host.setTxEnableDelay(100);

  for(int i = 0; i < sizeof(TELEGRAMS)/sizeof(TELEGRAMS[0]);i++){
    host.add_telegram(TELEGRAMS[i]);
  }
  ttn_otaa_init();

  rtc.begin();
  rtc.setAlarmSeconds(10);             
  rtc.attachInterrupt(alarmMatch);
  rtc.setMinutes(0);
  rtc.setSeconds(0);
  
  u8state = 6; //Start at sending stage so that we can send the initial package
  querying_count = host.getTelegramSize(); //
  //number of samples collected before we send over LoRa
  accumulate_count = 0xFF;//initialized to max as indicator that we are starting
  Serial.println("past setup()");
  rtc.enableAlarm(rtc.MATCH_SS); 

}

//have alarm be a setting where the alarm will trigger that it is time to sample again.

void loop() {
 
    switch(u8state){ 
          case 0:
            wdt_enable();
            new_tail = new queue_t;
            new_tail->buffer.push_back(Connection_Num);
            new_tail->buffer.push_back(rtc.getMinutes());
            new_tail->buffer.push_back(rtc.getSeconds());
            accumulate_count = SAMPLE_PERIOD-1; //Reset global variable
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
            host.poll(); // check incoming messages
            if (host.getState() == COM_IDLE) {
              ERR_LIST lastError = host.getLastError();
              u32wait = millis()+10;      
              if (host.getLastError() != ERR_SUCCESS) {
                Serial.print("Error ");
                Serial.println(int(lastError));
              } else {
                
                  process_data(host.getContainer(),host.getContainerCurrSize(),new_tail);  
                  /*for(int i = 0; i<new_tail->buffer.size();i++){
                    Serial.print(new_tail->buffer[i],HEX);Serial.print(" ");
                  }Serial.println(" "); */        
                  u8state = (querying_count==0) ? u8state+1 : 1;
                
                
              }
            }
            
            break;
          case 4:
            if(queue_count>2){
              for(int i =0;i<2;i++){
                queue_t * head = pop_front_queue();
                queue_count--;
                delete head;
              }
            }
            u8state++;
          break;
          case 5:
            if(SEND_COMPLETE && queue){
              delete[] mydata;
              queue_t* head = pop_front_queue();
              queue_count--;
              DATA_LENGTH = head->buffer.size();
              mydata = new uint8_t[DATA_LENGTH];
              std::copy ( head->buffer.begin(), head->buffer.end(), mydata );
              do_send(&sendjob);

              Serial.print("Sending.."); Serial.print(DATA_LENGTH);
              Serial.print(" --- Queue count: ");Serial.println(queue_count);
              delete head;
            }
          u8state++;
          rtc.enableAlarm(rtc.MATCH_SS); 
          break;
          
          case 6:
            os_runloop_once(); 
            wdt_disable();  
            if(FAILED){
              connectionReset(); 
            }
          break;
    }
};

/**
 * @brief
 * Interrupt handler for alarm ring. Used by RTC.
 *  
 * 
 */
void alarmMatch()
{
  rtc.disableAlarm();
  if(accumulate_count == 0xFF){
    if(SEND_COMPLETE)
      u8state = 0;
  }else if(accumulate_count == 0){
    //Assuming that new_tail is already created.
    queue_count++;
    push_tail(new_tail);
    Serial.print("Pushing tail, Queue count: "); Serial.println(queue_count);
    u8state = 0;
  }else{
    u8state = 1;
  }
  accumulate_count--;
  u32wait = millis()+10;
  querying_count = host.getTelegramSize();
  int t = rtc.getSeconds()+sample_rate;
  if(t>=60) t-=60;
  rtc.setAlarmSeconds(t);
  Serial.print(rtc.getMinutes());Serial.print(":");Serial.println(rtc.getSeconds());
}


/**
 * @brief
 * Resets connection when the radio becomes disconnected. This reruns the initalization code.
 * Will need to check if there is memory leakage.
 *  
 * 
 */
void connectionReset(){
    rtc.disableAlarm();
    ttn_otaa_init();
    rtc.setAlarmSeconds(rtc.getSeconds()+1); 
    rtc.enableAlarm(rtc.MATCH_SS);
}
