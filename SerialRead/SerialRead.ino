
#include "ttn-otaa.h"

void setup()
{
    Serial.begin(19200);
    Serial1.begin(19200);
    ttn_otaa_init();
}

void loop()
{
    os_runloop_once();
    if (FAILED)
    {
        setup();
    }

    if (SEND_COMPLETE)
    {
        int count;
        while (Serial1.available() > 0)
        {
            //reading
            Serial1.println(1); //ready
            Serial1.flush();

            count = Serial1.readBytes(mydata, 100);
            Serial.print("bytes read: ");
            Serial.println(count);
        }
        for (int i = 0; i < DATA_LENGTH; i++)
        {
            Serial.print(mydata[i]);
            Serial.print(",");
        }
        Serial.println("");
        //do_send(&sendjob);
    }
    else
    {
        Serial1.println(0); //!ready
    }
}