

/**
 * 3 CT's. Real and Reactive. 6 reading total.
 * Each reading is a 4 byte float. We can compress it to 2 bytes
 * Total is therefore 12 bytes per second. We can send up to 100 
 * bytes. This means every eight seconds we send 96 bytes. We 
 * can also send every 5 seconds for 60 bytes.
*/


void setup()
{
    Serial.begin(19200);
    Serial1.begin(19200);
    //while (!Serial || !Serial1){;}
    delay(10);
    Serial.print("begin");
}

void loop()
{
    Serial.println("sending");   
    Serial1.print('1');
    delay(1000);
    /*while(Serial1.available()){
        int a = Serial1.read();
    Serial.print("read: ");Serial.println(a);
    }*/
    
}
