/**
 * This is a modification of the simple_host example of the Modbus 
 * for Arduino library from MCCI.
 * This sketch uses a combination of interrupts and headers to poll data
 * from WattNode meters using Modbus, processes the data, and then sends it 
 * over LoRa to TTN (or other server).
 * 
 * Do not try to write to registers using this file. It will break the program
 * 
 * Comment Updated 8/10/2018
*/


#include <Catena.h>
#include "ttn-otaa.h"
#include "Catena_ModbusRtu.h"
#include <RTCZero.h>
#include <algorithm>
#include <utility>

#define kPowerOn        A3

using namespace McciCatena;


/** Initializing the modbus_t telegrams. There is an offset of 1
 * for the addresses we put into the initializer. The addresses we 
 * put into the telegram should be address on the manual minus 1. 
 * REGS (with offset): Each reg is 16 bits (2 bytes)
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
/**ASSUMPTION:
 * CT's for device 1 is 200A 
 * CT's for device 2 is 100A
 */
//User set variables
static const modbus_t T1 = {1,3,1010,4,nullptr}; //using only the first 2 phases of device 1
static const modbus_t T2 = {1,3,1148,4,nullptr};
static const modbus_t T3 = {2,3,1010,6,nullptr}; //using all 3 phases for device 2
static const modbus_t T4 = {2,3,1148,6,nullptr};

/**
 * Send order will be: 
 * Grid Power A real, Grid Power B real, 
 * Grid Power A Reactive, Grid Power B Reactive, 
 * AHU A real, AHU B/RTU B real, RTU A real,
 * AHU A Reactive, AHU B/RTU B Reactive, RTU A Reactive  
*/
static const modbus_t TELEGRAMS[] = {T1,T2,T3,T4}; //Order of transmitting


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
RTCZero rtc; //Real time clock for polling once a second

//Global Variables - user do not need to define.
volatile uint8_t u8state = 0; //FSM state
queue_t *new_tail = new queue_t; //queue containing the payload to be sent
volatile uint8_t querying_count=0; //count for number of telegrams need to query per second
volatile uint8_t accumulate_count=0; //count for size of payload to send
volatile uint8_t queue_count=0; //Length of the queue linked list
void connectionReset(); //Resets connection if somehow disconnected 
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
  rtc.setAlarmSeconds(5);             
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
            rtc.disableAlarm();
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
                delete new_tail;
                u8state = 0;
                querying_count = host.getTelegramSize();
                int t = rtc.getSeconds()+sample_rate;
                if(t>=60) t-=60;
                rtc.setAlarmSeconds(t);
              } else {
                
                  process_data(host.getContainer(),host.getContainerCurrSize(),new_tail);  
                  /*for(int i = 0; i<new_tail->buffer.size();i++){
                    Serial.print(new_tail->buffer[i],HEX);Serial.print(" ");
                  }Serial.println(" ");        */ 
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
            if(FAILED){
              connectionReset(); 
            }
            break;
    }
};

/**
 * @brief
 * Interrupt handler for alarm ring. Used by RTC. Updates alarm
 * This interrupt will trigger periodically set by SAMPLE_PERIOD
 * The interrupt prompts the meter to query and poll all the telegrams
 * through modbus. When the buffer has been filled (set by SAMPLE_PERIOD),
 * then the interrupt pushes the buffer onto the queue to be sent and 
 * prepares a new buffer.   
 * 
 */
void alarmMatch()
{
  
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
 * Resets connection when the radio becomes disconnected. 
 * This reruns the initalization code.
 * Will need to check if there is memory leakage.
 *  
 */
void connectionReset(){
    rtc.disableAlarm();
    ttn_otaa_init();
    rtc.setAlarmSeconds(rtc.getSeconds()+1); 
    rtc.enableAlarm(rtc.MATCH_SS);
}
