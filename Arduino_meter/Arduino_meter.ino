

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(A1,INPUT);
  pinMode(A2,INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  int value1 = analogRead(A1);
  int value2 = analogRead(A2);
  Serial.print(value1);
  Serial.println(value2);
}
