// EmonLibrary examples openenergymonitor.org, Licence GNU GPL V3

#include "EmonLib.h"                   // Include Emon Library
EnergyMonitor emon1;                   // Create an instance



void setup()
{  
  Serial.begin(115200);
  pinMode(A1, OUTPUT);
  //emon1.current(1, 600.6);             // Current: input pin, calibration.
}

void loop()
{
  //digitalWrite(8, HIGH);
  //double Irms = emon1.calcIrms(1480);  // Calculate Irms only
  double reading = analogRead(A1);
    //digitalWrite(8, LOW);
  //Serial.print(Irms*116.8/.7071);	       // Apparent power
  //Serial.print(" ");
  //Serial.println(Irms);		       // Irms
  Serial.println(reading);
  //emon1.serialprint();
  
}
