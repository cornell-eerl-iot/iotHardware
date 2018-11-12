

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
 * can also send 60 bytes every 5 seconds.
*/
#include <stdlib.h>

uint8_t *buff;
int size = 120;
int counter = 0;

void setup()
{
    Serial.begin(115200);
    Serial1.begin(115200);
    while (!Serial1)
        ;
    buff = new uint8_t[size];
    for(int i =0; i<size;i++){
        buff[i]=0;
    }
}


void loop()
{
    
    Serial1.print('<');
    Serial1.flush();
    while (Serial1.available()){
        if (Serial1.read()=='>'){
        //Serial.println("receiving messages");
            while (Serial1.available()){
                //int count = Serial.readBytes(buff, 1);
                
                char a [2]; 
                int count = Serial1.readBytes(a, 2);
                uint8_t number = strtol(a,NULL,16);
                //  Serial1.print(number,HEX);
                //  Serial1.print("|");
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
        
    delay(20);
    //serialClearBuffer();
    //delay(2000);
}   
