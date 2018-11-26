
/**
 * 3 CT's. Real and Reactive. 6 reading total.
 * Each reading is a 4 byte float. We can compress it to 2 bytes
 * Total is therefore 12 bytes per second. We can send up to 100 
 * bytes. This means every eight seconds we send 96 bytes. We 
 * can also send every 5 seconds for 60 bytes.
*/

#include "ttn-otaa.h"
#include <stdlib.h>

char *buff;
int size = 2;
bool ready = true;
int counter = 0;

void setup()
{
    Serial.begin(19200);
    Serial1.begin(115200);
    ttn_otaa_init();
    buff = new char[size];
    for(int i =0; i<size;i++){
        buff[i]=0;
    }
    counter = 0;
}

void loop()
{
    
    if (FAILED)
    {
        ttn_otaa_init();
    }

    if (SEND_COMPLETE)
    {
        if(Serial1.available()==0){ //&& ready){
            Serial1.print('<');
            Serial1.flush();
        }
        delay(100);
        counter = 0;
        while (Serial1.available())
        {
            //Serial.print("receiving messages");
            char a [size]; 
            int count = Serial1.readBytes(a, 1);
            for (int i = 0;i<size;i++){
                Serial.print(a[i]);
            }
            Serial.println("");
            uint8_t number = strtol(buff,NULL,16);
            
            mydata[counter++] = number;
            if (counter > DATA_LENGTH)
                counter = 0;
        }
        do_send(&sendjob);
    }
    else
    {
        os_runloop_once();
    }
    delay(100);
}