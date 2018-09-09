# 1 "c:\\Users\\xiaoy\\Documents\\GitHub\\iotHardware\\Arduino_meter\\Arduino_meter.ino"
# 1 "c:\\Users\\xiaoy\\Documents\\GitHub\\iotHardware\\Arduino_meter\\Arduino_meter.ino"


int V_DC_OFFSET = 2.5;
int I_DC_OFFSET = 2.5;

//CT: 200A:0.33VAC




void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(A1,0x0);
  pinMode(A0,0x0);
}

void loop() {
  // put your main code here, to run repeatedly:
  int current = analogRead(A1);
  int voltage = analogRead(A0);
  Serial.print(current);
  Serial.print(" ");
  Serial.println(voltage);
}
