
void setup()
{
    Serial.begin(9600);                             // serial data rate is set for 9600bps(bits per second)
    
    pinMode(13,OUTPUT);
    digitalWrite(13,LOW);
}
void loop()                                           // execute the loop forever
{
  
     while (Serial.available()) {
        String str1 = Serial.readStringUntil('!');
        String str2 = Serial.readStringUntil('!');
      
        long times = str1.toInt();
                  
        
        for(int i = 0;i<times;i++){
                digitalWrite(13,HIGH);
                delay(500);                                           // wait for 200 milli second
                digitalWrite(13,LOW);                          // turn OFF the LED
                delay(500);
        }
        Serial.println(str2);
     }
     
}
