//
// BobbinMotor.h
//

#ifndef _BOBINMOTOR_h
#define _BOBINMOTOR_h

#include "arduino.h"

//define PWM Parameter
/*
#define PWM_CH0	0
#define PWM_CH1	1
#define PWM_CH2	2
#define PWM_CH3	3
*/

// Move TO Common
//========================================
//#define BLDC_HZ 		22000		//22 Khz
//#define BLDC_RESOL10	10			// 10bit (0 ~1023)
//#define BLDC_RESOL8		8			// 8 bit (0 ~255)
//#define BLDC_RESOL		BLDC_RESOL10
//========================================

//---DUTY MAX
//#define BOBBIN_MAX_DUTY		255			// 8 bit
#define BOBBIN_MAX_DUTY		1023			// 10 bit
#define BOBBIN_OFF_DUTY	0


//==== [NOT USE]  DutyPercent 0 ~100% ,  => 10 Bit Resol 1 ~ 1023
#define PWM_MIN_DUTY_PER 1		// FW Limit 1
//#define PWM_MAX_DUTY_PER 100	// NG? - Pwm Out Singnal is Waving100%
//#define PWM_MAX_DUTY_PER 99		// FW Limit 299
#define PWM_MAX_DUTY_PER 50		// FW Limit 99

// PWM DUTY [ 0 ~ 255 ]
#define PWM_MIN_DUTY_1	1			// FW Limit 1
#define PWM_MIN_DUTY	0			// FW Limit 1

#if (BLDC_DUTY_FULL)
	#define PWM_MAX_DUTY 255		// FW Limit 255
#elif(BLDC_DUTY_LIMIT192)				// LIMIT
	#define PWM_MAX_DUTY 192		// FW Limit 192	Very Powerful, Limit 128 Power
#else
	#define PWM_MAX_DUTY 128		// FW Limit 255	Very Powerful, Limit 128 Power
#endif

#if (FUNC_FET_PROTECT)
// FET파손 대책
// 정의 삭제, 변수로 변경.
//#define PWM_FET_MAX_DUTY99 99		// FW Limit 100 FOR FET
#endif



#define PWM_ADJ_MAX200	(255*100/200)	//200% = duty/128
#define PWM_ADJ_MAX150	(255*100/150)	//150% = duty/170
#define PWM_ADJ_MAX100	255				//100% = duty/255
#define PWM_ADJ_MAX075	(255*125/100)	// 75% = duty /318
#define PWM_ADJ_MAX050	(255*150/100)	// 50% = duty /382

class BobbinMotor
{
  public:
    BobbinMotor();
    void init();

    void onBldcString(String _msg);
    void onBldc(int _dir, int _duty);
    void offBldc();
	short sbbnOut = 0;
	
  private:
    int defaultDuty;
	int outDuty;
};

//===  EXTERN VAR
extern unsigned long curr_ms_tick;

extern int gameStatus;
extern unsigned int	motTestAct;
extern unsigned int	motTestBbnIdx;

extern String STX_BBN_MOTOR;

extern short bldcLimitVal;		// BLDC LIMIT VAL, at Main

#endif
