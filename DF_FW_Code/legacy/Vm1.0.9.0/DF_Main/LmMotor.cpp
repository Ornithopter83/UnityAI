//
//
//

#include "Common.h"
#include "LmMotor.h"

LmMotor::LmMotor()
{
  
}

/*------------------------------------------------------------------
   INIT Setting of Servo Motor
------------------------------------------------------------------*/
void LmMotor::init()
{ 
#if	(IO_LM_MOT)
	// 1) SET Output  : NON Active LEVEL
  digitalWrite(LM_24VON_PIN, LM_24VOFF);		// BLDC_24V_ON OFF

  digitalWrite(LM_MOT_ON_PIN, LM_MOT_OFF);		// OFF
  digitalWrite(LM_MOT_DIR_PIN, LM_MOT_CW);		// CW default
  
  	// 2) PWM SETUP
  ledcSetup(LM_MOT_CH, BLDC_HZ, BLDC_RESOL);			// PWM Set ( Channel no, 22 Khz, 10 bit : 0~1023
  ledcAttachPin(LM_MOT_PWM_PIN, LM_MOT_CH);			// PWM Set ( Pin No, Channel no)

  //=== DUTY설정없으면 50% Duty 출력됨.
#if LM_PWM_INVERT
  ledcWrite(LM_MOT_CH, LM_MAX_DUTY);				// OFF PWM OUT
#else
  ledcWrite(LM_MOT_CH, LM_OFF_DUTY);				// OFF PWM OUT
#endif

  digitalWrite(LM_MOT_ON_PIN, LM_MOT_OFF);		// BLDC _ON OFF

#endif
}


//#if (EDIT_ING)
#if (1)

/*------------------------------------------------------------------
   SET Request Angle(STRING) of Bobbin Motor
   (include Change to INT from STRING)

       msg : $DO xx-yy,A,ppp%  xx- Cat no, yy -item no, A - action no, ppp - Para
------------------------------------------------------------------*/
void LmMotor::onBldcString(String _msg)
{

#if	(IO_LM_MOT)

  int iCat = _msg.substring(3,(3+2)).toInt(); 		//Category Code
  											// "-" 5th char (Skip)
  int iItem = _msg.substring(6,(6+2)).toInt(); 		//Item Code
  											// 8th : ","
  int iAct = _msg.substring(9,(9+1)).toInt(); 		//Act Code
  											// 10th : ","
  int iPara = _msg.substring(11,(11+3)).toInt(); 	//Para Code

  if( (3 != iCat) || (5 != iItem))	{ return;	}	//if Not  LM-MOT 3-5 , then Return , if NOT Bobbin Motor

  switch(iAct)
  {
  	case 0 :	// BLDC OFF
		offBldc();
		break;
	case 1 :	//CW duty(255)
		onBldc(BBN_MOT_CW, iPara);
		break;
	case 2 :	//CCW duty(255)
		onBldc(BBN_MOT_CCW, iPara);
		break;
	case 3:		// Repeat
		// TBD
		break;
	default:
		// Error
		break;
  }

#endif
}


void LmMotor::onBldc(int _dir, int _duty255)
{
	
#if	(IO_LM_MOT)

	//digitalWrite(BBN_MOT_ON_PIN, BBN_MOT_OFF);		// 1st OFF
	digitalWrite(LM_24VON_PIN, LM_24VON);		// BLDC_24V_ON ON

	digitalWrite(LM_MOT_DIR_PIN, _dir);			// SET DIR
   
	// Limit 0 ~99
	if(PWM_MIN_DUTY > _duty255) _duty255 = PWM_MIN_DUTY;
	else if(PWM_MAX_DUTY < _duty255) _duty255 = PWM_MAX_DUTY;
	else { }

	
  #if (0)
	  if( motTestAct && ( GAME_WAVE < gameStatus && GAME_RANDING > gameStatus) )	// Hooking ~ Success & TEST SET
	  {
		_duty255 = _duty255 * motTestBbnIdx / 20;	// 20 Step
	  }
  #endif


  
  #if (LM_PWM_INVERT)
  	//outDuty = (BOBBIN_MAX_DUTY - (BOBBIN_MAX_DUTY * _dutyP / PWM_ADJ_MAX075));	  // SET PWM [ INVERT DUTY]
  	outDuty = (LM_MAX_DUTY - (LM_MAX_DUTY * _duty255 / PWM_ADJ_MAX100));	  // SET PWM [ INVERT DUTY]
  #else
  	//outDuty = (BOBBIN_MAX_DUTY * _dutyP / PWM_ADJ_MAX075);	  // SET PWM 
  	outDuty = (LM_MAX_DUTY * _duty255 / PWM_ADJ_MAX100);	  // SET PWM 
  #endif
  	ledcWrite(LM_MOT_CH, outDuty);	  // SET PWM ( / 1023 )
  	digitalWrite(LM_MOT_ON_PIN, LM_MOT_ON); 	  // SET ON

	//-- LOG OUT
  #if (LOG_IO_LM_MOT)
	LogPrintln(" LG] LMmot ON dir=" + String(_dir) + ", " + String(_duty255)+"/255, "+String(outDuty)+"/1023");
  #endif

#endif	// IO_LM_MOT : END

}

void LmMotor::offBldc()
{

#if	(IO_LM_MOT)

	digitalWrite(LM_24VON_PIN, LM_24VOFF);		// BLDC_24V_ON OFF
  
	digitalWrite(LM_MOT_ON_PIN, LM_MOT_OFF);

	#if (LM_PWM_INVERT)
  		ledcWrite(LM_MOT_CH, LM_MAX_DUTY);
	#else
  		ledcWrite(LM_MOT_CH, LM_OFF_DUTY);
	#endif

  //-- LOG OUT
  #if (LOG_IO_LM_MOT)

	#if (LM_PWM_INVERT)
		LogPrintln(" LG] LMmot OFF: " + String(LM_MAX_DUTY));
	#else
		LogPrintln(" LG] LMmot OFF: " + String(LM_OFF_DUTY));
	#endif

  #endif		// LOG_IO

#endif		// IO_LM_MOT

}

#endif

