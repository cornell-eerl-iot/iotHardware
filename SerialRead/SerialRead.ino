
/**
 * 3 CT's. Real and Reactive. 6 reading total.
 * Each reading is a 4 byte float. We can compress it to 2 bytes
 * Total is therefore 12 bytes per second. We can send up to 100 
 * bytes. This means every eight seconds we send 96 bytes. We 
 * can also send every 5 seconds for 60 bytes.
*/

#include "ttn-otaa.h"
#include <stdlib.h>

bool DEBUG = true;

void setup()
{
    if (DEBUG)
        Serial.begin(19200);
    Serial1.begin(115200);
    ttn_otaa_init();
}

void loop()
{

    if (FAILED)
    {
        ttn_otaa_init();
    }

    if (SEND_COMPLETE)
    {
        if (Serial1.available() == 0)
        { //&& ready){
            // if (DEBUG)
            //     Serial.println("ready");
            Serial1.print('<');
            Serial1.flush();
        }
        delay(100);
        while (Serial1.available())
        {
            if (Serial1.read() == '>')
            {
                char number_str[2];
                Serial1.readBytes(number_str, 2);

                DATA_LENGTH = strtol(number_str, NULL, 16);
                Serial.print(F("Data Length: "));
                Serial.println(DATA_LENGTH);
                byte num_bytes = 0;
                while (num_bytes < DATA_LENGTH)
                {
                    //Serial.print("receiving messages");
                    char a[2];
                    int count = Serial1.readBytes(a, 2);
                    uint8_t number = strtol(a, NULL, 16);
                    // if (DEBUG){
                    //     for (int i = 0;i<2;i++){
                    //         Serial.print(a[i]);
                    //     }
                    //     Serial.println("");
                    // }
                    mydata[num_bytes++] = number;
                }
            }
            if (DEBUG)
            {
                for (int i = 0; i < DATA_LENGTH; i++)
                {
                    Serial.print(mydata[i], HEX);
                    Serial.print("|");
                }
                Serial.println("");
            }
            do_send(&sendjob);
        }
    }
    else
    {
        os_runloop_once();
    }
}