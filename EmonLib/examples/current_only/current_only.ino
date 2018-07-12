// EmonLibrary examples openenergymonitor.org, Licence GNU GPL V3

#include "EmonLib.h"                   // Include Emon Library
EnergyMonitor emon1;                   // Create an instance



void setup()
{  
  Serial.begin(115200);
  pinMode(8, OUTPUT);
  emon1.current(1, 300);             // Current: input pin, calibration.
  int count = 0;
}

void loop()
{
  //digitalWrite(8, HIGH);
  double Irms = emon1.calcIrms(1480);  // Calculate Irms only
  //double reading = analogRead(1);
    //digitalWrite(8, LOW);
  //Serial.print(Irms*120.0*.7071);	       // Apparent power
  //Serial.print(" ");
  Serial.println(Irms);		       // Irms
  //Serial.println(reading);
  //emon1.serialprint();

}
