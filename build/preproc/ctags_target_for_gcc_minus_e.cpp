# 1 "c:\\Users\\xiaoy\\Documents\\GitHub\\ECE-3400-Fall-2018\\lab01\\robot-followline\\robot-followline.ino"
# 1 "c:\\Users\\xiaoy\\Documents\\GitHub\\ECE-3400-Fall-2018\\lab01\\robot-followline\\robot-followline.ino"
# 2 "c:\\Users\\xiaoy\\Documents\\GitHub\\ECE-3400-Fall-2018\\lab01\\robot-followline\\robot-followline.ino" 2

//#include <cmath>

int servo_right_pin = 9;
int servo_left_pin = 10;

Servo servo_right;
Servo servo_left;

char ROBOT_SPEED;


void robot_init(){
  pinMode(servo_right_pin,0x1);
  pinMode(servo_left_pin,0x1);
  servo_right.attach(servo_right_pin);
  servo_left.attach(servo_left_pin);
}

void robot_forward(){
  servo_right.write(ROBOT_SPEED);
  servo_left.write(((ROBOT_SPEED-180)>0?(ROBOT_SPEED-180):-(ROBOT_SPEED-180)));
}

void robot_turn(int dir)
{
  if(dir){
    servo_right.write(0);
    servo_left.write(0);
  }else{
    servo_right.write(180);
    servo_left.write(180);
  }

}

void setup() {
  pinMode(13,0x1);
  delay(2000);
  digitalWrite(13,0x1);
  // put your setup code here, to run once:
  robot_init();
}


void loop() {
  // put your main code here, to run repeatedly:
  robot_forward();
  delay(1000);
  robot_turn(1);
  delay(1000);
}
