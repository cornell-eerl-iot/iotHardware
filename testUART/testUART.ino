

/**
 * 3 CT's. Real and Reactive. 6 reading total.
 * Each reading is a 4 byte float. We can compress it to 2 bytes
 * Total is therefore 12 bytes per second. We can send up to 100 
 * bytes. This means every eight seconds we send 96 bytes. We 
 * can also send every 5 seconds for 60 bytes.
*/



/**
 * 3 CT's. Real and Reactive. 6 reading total.
 * Each reading is a 4 byte float. We can compress it to 2 bytes
 * Total is therefore 12 bytes per second. We can send up to 100 
 * bytes. This means every eight seconds we send 96 bytes. We 
 * can also send every 5 seconds for 60 bytes.
*/
#include <stdlib.h>

uint8_t *buff;
int size = 100;
int counter = 0;

void setup()
{
    Serial.begin(9600);
    while (!Serial)
        ;
    buff = new uint8_t[size];
    for(int i =0; i<size;i++){
        buff[i]=0;
    }
}


void loop()
{
    Serial.print('<');
    //Serial.flush();
    while (Serial.available()){
        if (Serial.read()=='>'){
        //Serial.println("receiving messages");
            while (Serial.available()){
                //int count = Serial.readBytes(buff, 1);
                
                char a [2]; 
                int count = Serial.readBytes(a, 2);
                uint8_t number = strtol(a,NULL,16);
                 Serial.print(number,HEX);
                 Serial.print("|");
                buff[counter++] = number;
                
                //if(count){
                //    uint8_t number = strtol(buff,NULL,16);
                //    Serial.print(number);
                //    Serial.print("|");
                // for (int i = 0;i<1;i++){
                // Serial.print(a[i]);
                // }
            }
            for (int i = 0;i<counter-1;i++){
                Serial.print(buff[i],HEX);Serial.print(",");
            }
            counter = 0;
            Serial.println("");
        }
            
    }   
        
    delay(2000);
    //serialClearBuffer();
    //delay(2000);
}

void serialClearBuffer(){
    while (Serial.available()){
        char t = Serial.read();
    }
}