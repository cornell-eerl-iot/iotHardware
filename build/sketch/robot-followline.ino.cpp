#include <Arduino.h>
#line 1 "c:\\Users\\xiaoy\\Documents\\GitHub\\ECE-3400-Fall-2018\\lab01\\robot-followline\\robot-followline.ino"
#line 1 "c:\\Users\\xiaoy\\Documents\\GitHub\\ECE-3400-Fall-2018\\lab01\\robot-followline\\robot-followline.ino"
#include <Servo.h>

//#include <cmath>

int servo_right_pin = 9;
int servo_left_pin = 10;

Servo servo_right;
Servo servo_left;

char ROBOT_SPEED;


#line 14 "c:\\Users\\xiaoy\\Documents\\GitHub\\ECE-3400-Fall-2018\\lab01\\robot-followline\\robot-followline.ino"
void robot_init();
#line 21 "c:\\Users\\xiaoy\\Documents\\GitHub\\ECE-3400-Fall-2018\\lab01\\robot-followline\\robot-followline.ino"
void robot_forward();
#line 26 "c:\\Users\\xiaoy\\Documents\\GitHub\\ECE-3400-Fall-2018\\lab01\\robot-followline\\robot-followline.ino"
void robot_turn(int dir);
#line 38 "c:\\Users\\xiaoy\\Documents\\GitHub\\ECE-3400-Fall-2018\\lab01\\robot-followline\\robot-followline.ino"
void setup();
#line 47 "c:\\Users\\xiaoy\\Documents\\GitHub\\ECE-3400-Fall-2018\\lab01\\robot-followline\\robot-followline.ino"
void loop();
#line 14 "c:\\Users\\xiaoy\\Documents\\GitHub\\ECE-3400-Fall-2018\\lab01\\robot-followline\\robot-followline.ino"
void robot_init(){
  pinMode(servo_right_pin,OUTPUT);
  pinMode(servo_left_pin,OUTPUT);
  servo_right.attach(servo_right_pin);
  servo_left.attach(servo_left_pin);
}

void robot_forward(){
  servo_right.write(ROBOT_SPEED);
  servo_left.write(abs(ROBOT_SPEED-180));
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
  pinMode(LED_BUILTIN,OUTPUT);
  delay(2000);
  digitalWrite(LED_BUILTIN,OUTPUT);
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

