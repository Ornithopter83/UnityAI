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
  ledcWrite(BOBBIN_CH, BOBBIN_MAX_DUTY);					  // OFF PWM OUT

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

}


void BobbinMotor::onBldc(int _dir, int _duty255)
{
	int	inDuty = _duty255;

	String logMsg;

	//== 트러블, Duty 0시 24V OFF
	// Duty값 없음
	if(0 == _duty255)	// OFF
  	{
		digitalWrite(BBN_MOT_ON_PIN, BBN_MOT_OFF);		// 1st OFF
			digitalWrite(BBN_BLDC_24VON_PIN, BBN_BLDC_24VOFF); 	  // BLDC_24V_ON OFF
		
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
	//else if(PWM_FET_MAX_DUTY99 < _duty255) _duty255 = PWM_FET_MAX_DUTY99;	// FET 100 LIMIT
	else if(bldcLimitVal < _duty255) _duty255 = bldcLimitVal;	// FET 100 LIMIT
	else { }

	
	// 현재 REL 구성은 시험 배율을 적용하지 않는다.

	// 출력DUTY 저장
	sbbnOut = _duty255;

  
  	// 원본 REL: 명령 duty(0~255)를 10비트 반전 PWM(0~1023)으로 변환한다.
  	outDuty = BOBBIN_MAX_DUTY - (BOBBIN_MAX_DUTY * _duty255 / PWM_ADJ_MAX100);
  	ledcWrite(BOBBIN_CH, outDuty);	  // SET PWM ( / 1023 )
	// TBD ON/OFF Control 
	if(0 != _duty255)	// OFF
	{
	  	digitalWrite(BBN_MOT_ON_PIN, BBN_MOT_ON); 	  // SET ON
	}

	//-- LOG OUT
	//LogPrintln(" LG] BLDCo FW," + String(_dir) + "," + String(_duty255)+"/255, "+String(outDuty)+"/1023");
	logMsg = ((BBN_MOT_CW ==_dir)? "CW_" : "CCW");
	//LogPrintln(" LG] BLDCo FW," + ((BBN_MOT_CW ==_dir)? "CW_" : "CCW") + "," + String(_duty255) + "/255");
	LogPrintln(" LG] BLDCo " + logMsg + "," + String(_duty255) + "/255" + (_duty255 != inDuty ? "in:" + String(inDuty) : "")
		+ ",pwm_cmd:" + String(outDuty) + "/" + String(BOBBIN_MAX_DUTY)
		+ ",hz:" + String(ledcReadFreq(BOBBIN_CH)));


}

void BobbinMotor::offBldc()
{


		digitalWrite(BBN_BLDC_24VON_PIN, BBN_BLDC_24VOFF);		// BLDC_24V_ON OFF
  
	digitalWrite(BBN_MOT_ON_PIN, BBN_MOT_OFF);

  		ledcWrite(BOBBIN_CH, BOBBIN_MAX_DUTY);

	// 출력DUTY 저장
	sbbnOut = PWM_MIN_DUTY;

  //-- LOG OUT

		LogPrintln(" LG] BLDCo OFF: " + String(BOBBIN_MAX_DUTY));



}



