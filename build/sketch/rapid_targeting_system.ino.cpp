#include <Arduino.h>
#line 1 "C:\\Users\\xiaoy\\Documents\\Arduino\\rapid_targeting_system\\rapid_targeting_system.ino"
#line 1 "C:\\Users\\xiaoy\\Documents\\Arduino\\rapid_targeting_system\\rapid_targeting_system.ino"
#include "wifi.h"
#include "robot.h"

// Global constants for pin assignments
int pin_motor_left_dir = 5;
int pin_motor_right_dir = 10;
int pin_motor_left_speed = 3;
int pin_motor_right_speed = 9;
int pin_bump_right = 6;
int pin_bump_left = 7;
int pin_IR_bot = A4;
int pin_IR_front = A3;
int pin_led1=A2;
 // The setup routine runs once when you press reset

int sensor_value_bot;
int sensor_value_front;
int target_value=400;
int right_count=0;
int left_count=0;

int map_max=40;
int infra_map[40];
int counter=0;
int minimum_value=0xFFFF;
int switch_pressed_right;
int switch_pressed_left;

Robot robot1(pin_motor_left_dir,pin_motor_right_dir,
pin_motor_left_speed,pin_motor_right_speed,pin_bump_right,pin_bump_left);

#line 32 "C:\\Users\\xiaoy\\Documents\\Arduino\\rapid_targeting_system\\rapid_targeting_system.ino"
void setup();
#line 56 "C:\\Users\\xiaoy\\Documents\\Arduino\\rapid_targeting_system\\rapid_targeting_system.ino"
void loop();
#line 76 "C:\\Users\\xiaoy\\Documents\\Arduino\\rapid_targeting_system\\rapid_targeting_system.ino"
int state_machine_next_state(int current_state);
#line 162 "C:\\Users\\xiaoy\\Documents\\Arduino\\rapid_targeting_system\\rapid_targeting_system.ino"
int state_machine_switch(int state);
#line 233 "C:\\Users\\xiaoy\\Documents\\Arduino\\rapid_targeting_system\\rapid_targeting_system.ino"
void draw(int value);
#line 243 "C:\\Users\\xiaoy\\Documents\\Arduino\\rapid_targeting_system\\rapid_targeting_system.ino"
void analyze();
#line 32 "C:\\Users\\xiaoy\\Documents\\Arduino\\rapid_targeting_system\\rapid_targeting_system.ino"
void setup() {
  Serial.begin(9600);
  pinMode(pin_led1, OUTPUT);
  analogWrite(pin_led1,255);
  delay(1000);
  analogWrite(pin_led1,0);
  wifi_init();
  robot1.curr_state = ROBOT_START;
  
  //int number;
  GOT_NUMBER = false; //check if it works with different numbers
  while(!GOT_NUMBER){
    analogWrite(pin_led1,255);
    robot1.spins = get_number();
  }
  Serial.println("");Serial.print("file number ");Serial.print(robot1.spins);
  analogWrite(pin_led1,0);
  
  map_max = robot1.spins;
  
}

 // The loop routine runs over and over again

void loop() { 
  
  sensor_value_bot = analogRead(pin_IR_bot);
  sensor_value_front = analogRead(pin_IR_front);
  /*Serial.print("Bottom sensor = ");
  Serial.println(sensor_value_bot);
  */
  Serial.print("Front sensor = ");
  Serial.println(sensor_value_front);
  
  
  // switch state if necessary  
  Serial.print("current state: ");Serial.println(robot1.curr_state);
  //int next_state = state_machine_next_state(robot1.curr_state);
  state_machine_switch(robot1.curr_state);
  robot1.curr_state = state_machine_next_state(robot1.curr_state);
  
  //Serial.println("");
}

int state_machine_next_state(int current_state)
{ 
  int next_state = 0;
  
  

  switch(current_state)
  {
      case ROBOT_START:
      {
        if(robot1.spins>1){
          next_state = ROBOT_START;
          robot1.spins--;
        }else{
          analyze();
          next_state = ROBOT_FORWARD;
        }
        break;
      }
      case ROBOT_FORWARD: //looking state; robot will always go forward when looking
      {
        switch_pressed_right = digitalRead(pin_bump_right); // read digital value
        switch_pressed_left  = digitalRead(pin_bump_left); // read digital value
        next_state = ROBOT_FORWARD;
        if (switch_pressed_right || switch_pressed_left || sensor_value_front>=600)
        {
          next_state = ROBOT_HIT;
        }
        
        if((sensor_value_bot-target_value<40) && (sensor_value_bot-target_value>-40))
        {
          next_state = ROBOT_FOUND;
        } //gives us a range of values for target*/
        
        break;
      }
      case ROBOT_HIT:
      {
        next_state = ROBOT_REVERSE; break;
      }
      case ROBOT_REVERSE:
      {
        if (switch_pressed_right && switch_pressed_left || sensor_value_front>=600)
        {
          next_state = ROBOT_TURN;;
        }else if(switch_pressed_left)
        {  
          next_state = ROBOT_RIGHT;
        }else if(switch_pressed_right)
        {
          next_state = ROBOT_LEFT; 
        }
         break;
      }
      case ROBOT_TURN:
      {
        next_state = ROBOT_FORWARD; break;
      }
      
      case ROBOT_FOUND:
      {
        next_state = ROBOT_STOP;break;
      }
      
      case ROBOT_RIGHT:
      {
        right_count++;
        next_state = ROBOT_FORWARD; break;
      }
      case ROBOT_LEFT:
      {
        left_count++;
        next_state = ROBOT_FORWARD; break;
      }
      case ROBOT_STOP:
      {
        next_state = ROBOT_STOP;break;
      }
      default:
      {
        Serial.println("state_machine_next_state - invalid state");
      }
  }
  return next_state;
}
// state switch
int state_machine_switch(int state)
{
  switch(state)
  {
    case ROBOT_START:
    {
      draw(sensor_value_front);
      robot1.turn(right, robot1.get_turn_time(), 110);
      //robot1.spin();break;
    }
    case ROBOT_HIT:
    {
      
      break;
    }
    case ROBOT_FORWARD: 
    {
      robot1.forward(); break;
    }
    case ROBOT_REVERSE: 
    {
      robot1.reverse(); break;
    }
    case ROBOT_TURN: 
    {
      /*
        if(right_count<=left_count*3){
          robot1.turn(right); 
          right_count++;
        }else{
          robot1.turn(left);
          left_count++;
        }
      */
      robot1.turn(left);
      int left = analogRead(pin_IR_front);
      robot1.turn(right);
      robot1.turn(right);
      int right = analogRead(pin_IR_front);
      if(right>left){
        robot1.turn(left);
        robot1.turn(left);
      }
      break;
    }
  	case ROBOT_FOUND:
  	{
  		robot1.spin();break;
  	}
    case ROBOT_RIGHT:
    {
      //robot1.reverse();
      robot1.turn(right);break;
    }
    case ROBOT_LEFT:
    {
      //robot1.reverse();
      robot1.turn(left);break;
    }
    case ROBOT_STOP:
    {
      robot1.stop_m();break;
    }
    default:
      {
        Serial.println("state_machine_switch - invalid state");
      }
  }
  return state;
}

void draw(int value){
	if(counter >= map_max){
		Serial.println("MAX SIZE MAP REACHED");
	}else{
		if(value<minimum_value) minimum_value=value;
		infra_map[counter]=value;
		counter++;
	}
}

void analyze(){
  for (int i=0;i<map_max;i++){
    Serial.print(infra_map[i]);Serial.print(" ");
  }
  Serial.println("");
	while (counter>0){
		if(infra_map[counter]<=minimum_value){
      counter = 0;
		}else{
      robot1.turn(left, robot1.get_turn_time(), 110);
		}
    counter--;
	}
}

