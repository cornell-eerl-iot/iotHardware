
#include <Catena.h>
#include "ttn-otaa.h"
#include "Catena_ModbusRtu.h"
#include <RTCZero.h>
#include <algorithm>

#define kPowerOn        A3

using namespace McciCatena;


Catena gCatena;
RTCZero rtc;
// data array for modbus network sharing

//User set variables
static const modbus_t T1 = {1,3,1701,6,nullptr};
static const modbus_t T2 = {1,3,1707,6,nullptr};
static const modbus_t T3 = {1,3,1601,4,nullptr};
static const modbus_t T4 = {1,3,1605,4,nullptr};

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
queue_t *new_tail;
volatile uint8_t querying_count=0;
volatile uint8_t accumulate_count=0;
volatile int queue_count=0;
volatile uint8_t t=0; //time counter for RTC interrupts

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

  for(int i = 0; i < sizeof(TELEGRAMS)/sizeof(TELEGRAMS[0]);i++){
    host.add_telegram(TELEGRAMS[i]);
  }
  host.print_telegrams();
  ttn_otaa_init();

  do_send(&sendjob); //establish connection

  //Wait until we are able to join the network before start polling.
  while(!JOINED){// Just want to join, will send in FSM.
    os_runloop_once();   
  }
  rtc.begin();
  t=5;
  rtc.setAlarmSeconds(t);             
  rtc.attachInterrupt(alarmMatch);
  rtc.setSeconds(0);
  rtc.enableAlarm(rtc.MATCH_SS);

  u8state = 6; //Start at sending stage so that we can send the initial package
  u32wait = millis();
  querying_count = host.getTelegramSize(); //
  accumulate_count = 1;//number of samples collected before we send over LoRa
  Serial.println("past setup()");
}

//have alarm be a setting where the alarm will trigger that it is time to sample again.

void loop() {
 
    switch(u8state){ 
          case 0:
            new_tail = new queue_t;
            accumulate_count = SAMPLE_PERIOD; //Reset global variable
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
    queue_count++;
    push_tail(new_tail);
    Serial.print("pushing tail, queue count: "); Serial.println(queue_count);
    u8state = 0;
  }else{
    u8state = 1;
  }
  u32wait = millis()+10;
  querying_count = host.getTelegramSize();
  t+=SAMPLE_RATE;
  if(t>=60) t-=60;
  rtc.setAlarmSeconds(t);
  Serial.println(t);
}
