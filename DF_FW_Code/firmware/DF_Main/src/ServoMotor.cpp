//
//
//

#include "Common.h"
#include "ServoMotor.h"


ServoMotor::ServoMotor()
{
  
}

/*------------------------------------------------------------------
   INIT Setting of Servo Motor
------------------------------------------------------------------*/
void ServoMotor::init()
{  
}

/*------------------------------------------------------------------
   SET Request Angle(STRING) of Servo Motor
   (include Change to INT from STRING)
       OutputAngle = 90 + ( -+ Req Angle)
------------------------------------------------------------------*/
void ServoMotor::setAngle(String _data)
{
}

//
// SET INT
//
void ServoMotor::setAngle(int _val)
{
}

/*------------------------------------------------------------------
   SET Default Angle(90 degree) of Servo Motor
------------------------------------------------------------------*/
void ServoMotor::setAngleDefault()
{
}
