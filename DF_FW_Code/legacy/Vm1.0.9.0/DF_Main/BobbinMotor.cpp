//
//
//

#include "Common.h"
#include "BobbinMotor.h"

BobbinMotor::BobbinMotor()
{
  
}

/*------------------------------------------------------------------
   INIT Setting of Servo Motor
------------------------------------------------------------------*/
void BobbinMotor::init()
{ 

	// 1) SET Output  : NON Active LEVEL
  digitalWrite(BBN_BLDC_24VON_PIN, BBN_BLDC_24VOFF);		// BLDC_24V_ON OFF

  digitalWrite(BBN_MOT_ON_PIN, BBN_MOT_OFF);		// OFF
  digitalWrite(BBN_MOT_DIR_PIN, BBN_MOT_CW);		// CW default
  
  	// 2) PWM SETUP
  ledcSetup(BOBBIN_CH, BLDC_HZ, BLDC_RESOL);		// PWM Set ( Channel no, 22 Khz, 10 bit : 0~1023
  ledcAttachPin(BBN_MOT_PWM_PIN, BOBBIN_CH);				// PWM Set ( Pin No, Channel no)

  //=== DUTY설정없으면 50% Duty 출력됨.
#if BBN_PWM_INVERT
  ledcWrite(BOBBIN_CH, BOBBIN_MAX_DUTY);					// OFF PWM OUT
#else
  ledcWrite(BOBBIN_CH, BOBBIN_OFF_DUTY);					  // OFF PWM OUT
#endif

	// 출력DUTY 저장
	sbbnOut = PWM_MIN_DUTY;

	digitalWrite(BBN_MOT_ON_PIN, BBN_MOT_OFF);		// BLDC _ON OFF

}

/*------------------------------------------------------------------
   SET Request Angle(STRING) of Bobbin Motor
   (include Change to INT from STRING)

       msg : $DO xx-yy,A,ppp%  xx- Cat no, yy -item no, A - action no, ppp - Para
------------------------------------------------------------------*/
void BobbinMotor::onBldcString(String _msg)
{

#if	(IO_BBN)

  int iCat = _msg.substring(3,(3+2)).toInt(); 		//Category Code
  											// "-" 5th char (Skip)
  int iItem = _msg.substring(6,(6+2)).toInt(); 		//Item Code
  											// 8th : ","
  int iAct = _msg.substring(9,(9+1)).toInt(); 		//Act Code
  											// 10th : ","
  int iPara = _msg.substring(11,(11+3)).toInt(); 	//Para Code

  if( (3 != iCat) || (3 != iItem))	{ return;	}	//if Not  BLDC 3-3 , then Return , if NOT Bobbin Motor

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


void BobbinMotor::onBldc(int _dir, int _duty255)
{
	int	inDuty = _duty255;
#if	(IO_BBN)

	String logMsg;

	//== 트러블, Duty 0시 24V OFF
	// Duty값 없음
	if(0 == _duty255)	// OFF
  	{
		digitalWrite(BBN_MOT_ON_PIN, BBN_MOT_OFF);		// 1st OFF
		#if (TEST_BLDC24V_FORCE_ON)	// TEST = 24V_ALWAYS_ON TEST
			digitalWrite(BBN_BLDC_24VON_PIN, BBN_BLDC_24VON);	  // BLDC_24V_ON OFF
		#else				// NORMAL = REL
			digitalWrite(BBN_BLDC_24VON_PIN, BBN_BLDC_24VOFF); 	  // BLDC_24V_ON OFF
		#endif
		
  	}
  	else // Duty값 있음, DUTY != 0, 24V ON
  	{
		digitalWrite(BBN_BLDC_24VON_PIN, BBN_BLDC_24VON);		// BLDC_24V_ON ON
  	}

	// == 이하 , 방향, Duty설정
	digitalWrite(BBN_MOT_DIR_PIN, _dir);			// SET DIR
   
//    -(minus 값) 체크
	if(PWM_MIN_DUTY > _duty255) _duty255 = PWM_MIN_DUTY;

// === 제한값 범위 Check,  BLDC DUTY LIMIT 0 ~ 100 for FET
#if (FUNC_FET_PROTECT)
	//else if(PWM_FET_MAX_DUTY99 < _duty255) _duty255 = PWM_FET_MAX_DUTY99;	// FET 100 LIMIT
	else if(bldcLimitVal < _duty255) _duty255 = bldcLimitVal;	// FET 100 LIMIT
#else
	else if(PWM_MAX_DUTY < _duty255) _duty255 = PWM_MAX_DUTY;		// TEST(버티기)
#endif
	else { }

	
  #if (MKT_TEST_3)
	  if( motTestAct && ( GAME_WAVE < gameStatus && GAME_RANDING > gameStatus) )	// Hooking ~ Success & TEST SET
	  {
		_duty255 = _duty255 * motTestBbnIdx / 20;	// 20 Step
	  }
  #endif

	// 출력DUTY 저장
	sbbnOut = _duty255;

  
  #if BBN_PWM_INVERT
  	//outDuty = (BOBBIN_MAX_DUTY - (BOBBIN_MAX_DUTY * _dutyP / PWM_ADJ_MAX075));	  // SET PWM [ INVERT DUTY]
  	outDuty = (BOBBIN_MAX_DUTY - (BOBBIN_MAX_DUTY * _duty255 / PWM_ADJ_MAX100));	  // SET PWM [ INVERT DUTY]
  #else
  	//outDuty = (BOBBIN_MAX_DUTY * _dutyP / PWM_ADJ_MAX075);	  // SET PWM 
  	outDuty = (BOBBIN_MAX_DUTY * _duty255 / PWM_ADJ_MAX100);	  // SET PWM 
  #endif
  	ledcWrite(BOBBIN_CH, outDuty);	  // SET PWM ( / 1023 )
	// TBD ON/OFF Control 
	if(0 != _duty255)	// OFF
	{
	  	digitalWrite(BBN_MOT_ON_PIN, BBN_MOT_ON); 	  // SET ON
	}

	//-- LOG OUT
  #if (LOG_IO_BBN)
	//LogPrintln(" LG] BLDCo FW," + String(_dir) + "," + String(_duty255)+"/255, "+String(outDuty)+"/1023");
	logMsg = ((BBN_MOT_CW ==_dir)? "CW_" : "CCW");
	//LogPrintln(" LG] BLDCo FW," + ((BBN_MOT_CW ==_dir)? "CW_" : "CCW") + "," + String(_duty255) + "/255");
	LogPrintln(" LG] BLDCo " + logMsg + "," + String(_duty255) + "/255" + (_duty255 != inDuty ? "in:" + String(inDuty) : ""));
  #endif

#endif	// IO_BBN : END

}

void BobbinMotor::offBldc()
{

#if	(IO_BBN)

	#if (TEST_BLDC24V_FORCE_ON)	// TEST = 24V_ALWAYS_ON TEST
		digitalWrite(BBN_BLDC_24VON_PIN, BBN_BLDC_24VON);		// BLDC_24V_ON OFF
	#else		// NORMAL = REL
		digitalWrite(BBN_BLDC_24VON_PIN, BBN_BLDC_24VOFF);		// BLDC_24V_ON OFF
	#endif
  
	digitalWrite(BBN_MOT_ON_PIN, BBN_MOT_OFF);

	#if BBN_PWM_INVERT
  		ledcWrite(BOBBIN_CH, BOBBIN_MAX_DUTY);
	#else
  		ledcWrite(BOBBIN_CH, BOBBIN_OFF_DUTY);
	#endif

	// 출력DUTY 저장
	sbbnOut = PWM_MIN_DUTY;

  //-- LOG OUT
  #if (LOG_IO_BBN)

	#if BBN_PWM_INVERT
		LogPrintln(" LG] BLDCo OFF: " + String(BOBBIN_MAX_DUTY));
	#else
		LogPrintln(" LG] BLDCo OFF: " + String(BOBBIN_OFF_DUTY));
	#endif

  #endif		// LOG_IO

#endif		// IO_BBN

}



