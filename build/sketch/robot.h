#include <Servo.h>
// need to fill this out


enum ROBOT_STATES {
    ROBOT_START = 0,
    ROBOT_HIT = 1,
    ROBOT_FOUND = 2,
    ROBOT_FORWARD = 3,
    ROBOT_RIGHT = 4,
    ROBOT_LEFT = 5,
	ROBOT_REVERSE = 6,
	ROBOT_TURN = 7,
 ROBOT_STOP = 8,
};


enum DIRECTIONS {
	right = 0,
	left = 1,
	neither = -1
};

class Robot
{
	int turn_90 = 790; //in ms
	int turn_180 = 500;
  int turn_time = 300;
	int right_speed = 100;
	int left_speed = 100;
	int turn_direction;

	int pin_motor_left_dir = 12;
	int pin_motor_right_dir = 13;
	int pin_motor_left_speed = 3;
	int pin_motor_right_speed = 11;
	int pin_bump_right = 6;
	int pin_bump_left = 7;
	
	

public:
	uint8_t curr_state;
  int spins;
	Robot();
	Robot(int a, int b, int c, int d,int e, int f);
	void forward();
	void stop_m();
	void turn(DIRECTIONS r_direction);
	void turn(DIRECTIONS r_direction, int r_time, int r_speed);
	void reverse();
	void spin();
	void set_turn_time(int t){turn_time = t;}
	void set_right_speed(int s){right_speed = s;}
	void set_left_speed(int s){left_speed = s;} 
	void set_turn_direction(DIRECTIONS dir){turn_direction = dir;}
	
	DIRECTIONS get_turn_direction(){return turn_direction;}
	int get_turn_time(){return turn_time;}
}; 

Robot::Robot(){
	pinMode( pin_bump_right, INPUT );
	pinMode( pin_bump_left, INPUT );
	pinMode( pin_motor_left_dir, OUTPUT );
	pinMode( pin_motor_right_dir, OUTPUT );
	pinMode( pin_motor_left_speed, OUTPUT );
	pinMode( pin_motor_right_speed, OUTPUT );
	//init status
	digitalWrite( pin_motor_left_dir,   LOW);
	digitalWrite( pin_motor_right_dir,  LOW);
	analogWrite( pin_motor_left_speed,  0);
	analogWrite( pin_motor_right_speed, 0);
  
}

Robot::Robot(int a, int b, int c, int d,int e, int f): 
pin_motor_left_dir(a), pin_motor_right_dir(b), pin_motor_left_speed(c),
pin_motor_right_speed(d),pin_bump_right(e),pin_bump_left(f) {
	
	Robot();
}

void Robot::forward(){
  set_right_speed(155);
  set_left_speed(145);
	digitalWrite( pin_motor_left_dir,   LOW);
	digitalWrite( pin_motor_right_dir,  LOW);
	analogWrite( pin_motor_left_speed,  left_speed);
	analogWrite( pin_motor_right_speed, right_speed);
  set_right_speed(100);
  set_left_speed(100);
}

void Robot::stop_m(){
	digitalWrite( pin_motor_left_dir,   LOW);
	digitalWrite( pin_motor_right_dir,  LOW);
	analogWrite( pin_motor_left_speed,  0);
	analogWrite( pin_motor_right_speed, 0);
}

void Robot::turn(DIRECTIONS r_direction){
	set_right_speed(110);
  set_left_speed(110);
	if(r_direction == right){ 
		digitalWrite(pin_motor_left_dir, LOW);
		analogWrite(pin_motor_left_speed, left_speed);
		digitalWrite(pin_motor_right_dir, HIGH);
		analogWrite(pin_motor_right_speed, right_speed);
	}else{
		digitalWrite(pin_motor_left_dir, HIGH);
		analogWrite(pin_motor_left_speed, left_speed);
		digitalWrite(pin_motor_right_dir, LOW);
		analogWrite(pin_motor_right_speed, right_speed);
	}
	delay(turn_90);
	stop_m();
}

void Robot::turn(DIRECTIONS r_direction, int r_time, int r_speed){
	if(r_direction == right){
		digitalWrite(pin_motor_left_dir, HIGH);
		analogWrite(pin_motor_left_speed, r_speed);
		digitalWrite(pin_motor_right_dir, LOW);
		analogWrite(pin_motor_right_speed, r_speed);
	}else{
		digitalWrite(pin_motor_left_dir, LOW);
		analogWrite(pin_motor_left_speed, r_speed);
		digitalWrite(pin_motor_right_dir, HIGH);
		analogWrite(pin_motor_right_speed, r_speed);
	}
	delay(r_time);
}

void Robot::reverse(){
  set_right_speed(100);
  set_left_speed(100);
	digitalWrite(pin_motor_left_dir, HIGH);
	analogWrite(pin_motor_left_speed, left_speed);
	digitalWrite(pin_motor_right_dir, HIGH);
	analogWrite(pin_motor_right_speed, right_speed);
	delay(turn_180);
	stop_m();
}

void Robot::spin(){
	for(int i=0;i<spins*4;i++){
		turn(right);
	}
}


