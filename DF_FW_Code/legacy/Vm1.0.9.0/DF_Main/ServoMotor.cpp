//
//
//

#include "Common.h"
#include "ServoMotor.h"

#if IO_SERVO
Servo servo;					// Servo Object Create
#endif

ServoMotor::ServoMotor()
{
  
}

/*------------------------------------------------------------------
   INIT Setting of Servo Motor
------------------------------------------------------------------*/
void ServoMotor::init()
{  
#if IO_SERVO
  ESP32PWM::allocateTimer(0);		// PWM Time_0 assign
  ESP32PWM::allocateTimer(1);		// PWM Time_1 assign
  ESP32PWM::allocateTimer(2);		// PWM Time_2 assign
  ESP32PWM::allocateTimer(3);		// PWM Time_3 assign
  // servo.setPeriodHertz(330);    		//Set 330 Hz(3.03ms), standard 50 hz servo
  servo.setPeriodHertz(330);    		//Set 330 Hz(3.03ms), standard 50 hz servo
  // Angle : -45 ~ +45
  //servo.attach(SRV_PWM_PIN, 1000, 2000);	//-45 ~ +45, Servo Attach [ Pin, Pulse Width Min us = 1ms, Max us = 2ms ]
  //servo.attach(SRV_PWM_PIN,  500, 2500);	//-90 ~ +90, Servo Attach [ Pin, Pulse Width Min us = 1ms, Max us = 2ms ]
  //servo.attach(SRV_PWM_PIN,  778, 2222);	//-65 ~ +65, Servo Attach [ Pin, Pulse Width Min us = 1ms, Max us = 2ms ]
  servo.attach(SERVOPWM_PIN,  834, 2166);	//-60 ~ +60, Servo Attach [ Pin, Pulse Width Min us = 1ms, Max us = 2ms ]
    
  defaultAngle = 90;				// Set Default Angle 90
  servo.write(defaultAngle);

  /* TEST
  defaultAngle = 0;				// Set Default Angle 90
  servo.write(defaultAngle);
  delay(3000);

  servo.write(180);
  delay(100);
  servo.write(0);
  delay(500);
  servo.write(90);
  */
#endif
}

/*------------------------------------------------------------------
   SET Request Angle(STRING) of Servo Motor
   (include Change to INT from STRING)
       OutputAngle = 90 + ( -+ Req Angle)
------------------------------------------------------------------*/
void ServoMotor::setAngle(String _data)
{
#if IO_SERVO

 #if (NEW_IF)
  int angle = _data.substring(3).toInt(); 		// 9th Char~11th Char ( -90 ~ +090)
 #else
  int angle = _data.substring(8).toInt(); 		// 9th Char~11th Char ( -90 ~ +090)
 #endif
 
  int mapAngle = 0;

#if HAND_TEST_01	// Servo Test
	//Test Angle -| dec |

  if( 0 > angle )
  {
  	mapAngle = map(angle, 0, -90, 0, (maxAdjAngle*-1));	//  ex -50 : -50 + 30 => -20 
  }
  else if( 0 < angle )
  {
  	mapAngle = map(angle, 0, 90, 0, (maxAdjAngle));	//  ex -50 : -50 + 30 => -20 
  }
  angle = mapAngle;

#endif
  angle += defaultAngle;					// 90 + ReqAngle( -90 ~ + 90)
  
  servo.write(angle);

#endif
}

//
// SET INT
//
void ServoMotor::setAngle(int _val)
{
#if IO_SERVO

	//-- TBD , HAND_TEST_01 Control
		// val = Map(val);
	//---
  int angle = _val + defaultAngle;					// 90 + ReqAngle( -90 ~ + 90)

  servo.write(angle);

#endif
}

/*------------------------------------------------------------------
   SET Default Angle(90 degree) of Servo Motor
------------------------------------------------------------------*/
void ServoMotor::setAngleDefault()
{
#if IO_SERVO
  servo.write(defaultAngle);
#endif
}
