//
//
//

#include "Common.h"
#include "LmMotor.h"
#include "DF_Main_State.h"

LmMotor::LmMotor()
{
  
}

/*------------------------------------------------------------------
   INIT Setting of Servo Motor
------------------------------------------------------------------*/
void LmMotor::init()
{ 
	digitalWrite(LM_24VON_PIN, LM_24VOFF);
	digitalWrite(LM_MOT_ON_PIN, LM_MOT_OFF);
	digitalWrite(LM_MOT_DIR_PIN, LM_MOT_CW);
	ledcSetup(LM_MOT_CH, BLDC_HZ, BLDC_RESOL);
	ledcAttachPin(LM_MOT_PWM_PIN, LM_MOT_CH);
	ledcWrite(LM_MOT_CH, LM_MAX_DUTY);
}



/*------------------------------------------------------------------
   SET Request Angle(STRING) of Bobbin Motor
   (include Change to INT from STRING)

       msg : $DO xx-yy,A,ppp%  xx- Cat no, yy -item no, A - action no, ppp - Para
------------------------------------------------------------------*/
void LmMotor::onBldcString(String _msg)
{
	int act = _msg.substring(9, 10).toInt();
	int duty = _msg.substring(11, 14).toInt();
	if(1 == act) { onBldc(LM_MOT_CW, duty); }
	else if(2 == act) { onBldc(LM_MOT_CCW, duty); }
	else { offBldc(); }
}


void LmMotor::onBldc(int _dir, int _duty255)
{
	if(DF_CONFIG_LMJIG != dfConfig) { return; }
	digitalWrite(LM_24VON_PIN, LM_24VON);
	digitalWrite(LM_MOT_DIR_PIN, _dir);
	if(PWM_MIN_DUTY > _duty255) { _duty255 = PWM_MIN_DUTY; }
	else if(PWM_MAX_DUTY < _duty255) { _duty255 = PWM_MAX_DUTY; }
	outDuty = LM_MAX_DUTY - (LM_MAX_DUTY * _duty255 / PWM_ADJ_MAX100);
	ledcWrite(LM_MOT_CH, outDuty);
	digitalWrite(LM_MOT_ON_PIN, LM_MOT_ON);
}

void LmMotor::offBldc()
{
	if(DF_CONFIG_LMJIG != dfConfig) { return; }
	digitalWrite(LM_24VON_PIN, LM_24VOFF);
	digitalWrite(LM_MOT_ON_PIN, LM_MOT_OFF);
	ledcWrite(LM_MOT_CH, LM_MAX_DUTY);
}


