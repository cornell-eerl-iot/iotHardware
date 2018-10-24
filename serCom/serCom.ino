
void setup()
{
    Serial.begin(19200);
    Serial1.begin(19200);
    while (!Serial)
        ;
}

void loop()
{
    //Serial.print('1'); //ready
    //Serial.flush();
    while (Serial.available() > 0)
    {
        //reading
        Serial.print(6);
        char *buff = new char[10]; 
        int count = Serial.readBytes(buff, 12);
        for (int i = 0; i < 10; i++)
        {
            Serial.print(buff[i]);
        }
    }
}