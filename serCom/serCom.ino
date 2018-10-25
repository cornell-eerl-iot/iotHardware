

/**
 * 3 CT's. Real and Reactive. 6 reading total.
 * Each reading is a 4 byte float. We can compress it to 2 bytes
 * Total is therefore 12 bytes per second. We can send up to 100 
 * bytes. This means every eight seconds we send 96 bytes. We 
 * can also send every 5 seconds for 60 bytes.
*/

#include <stdlib.h>

char *buff;
int size = 2;
bool ready = true;

void setup()
{
    Serial.begin(19200);
    Serial1.begin(19200);
    while (!Serial)
        ;
    buff = new char[size];
    for(int i =0; i<size;i++){
        buff[i]=0;
    }
}

void loop()
{
    while (Serial.available())
    {
        int count = Serial.readBytes(buff, 1);
        if(count){
            uint8_t number = strtol(buff,NULL,16);
            Serial.print(number);
            Serial.print("|");
        }
    }
    delay(2000);
    if(Serial.available()==0){ //&& ready){
        Serial.print('<');
        Serial.flush();
    }
}