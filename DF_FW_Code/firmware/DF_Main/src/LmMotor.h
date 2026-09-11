//
// LM( Linear Motor ).h
//

#pragma once

#include "arduino.h"

//define PWM Parameter
/*
#define PWM_CH0	0
#define PWM_CH1	1
#define PWM_CH2	2
#define PWM_CH3	3
*/

//---DUTY MAX
#define LM_MAX_DUTY		1023			// 10 bit
#define LM_OFF_DUTY		0


//==== [NOT USE]  DutyPercent 0 ~100% ,  => 10 Bit Resol 1 ~ 1023
#define PWM_MIN_DUTY_PER 1		// FW Limit 1
#define PWM_MAX_DUTY_PER 50		// FW Limit 99

// PWM DUTY [ 0 ~ 255 ]
#define PWM_MIN_DUTY_1	1			// FW Limit 1
#define PWM_MIN_DUTY	0			// FW Limit 1

	#define PWM_MAX_DUTY 192		// FW Limit 255	Very Powerful, Limit 128 Power


#define PWM_ADJ_MAX200	(255*100/200)	//200% = duty/128
#define PWM_ADJ_MAX150	(255*100/150)	//150% = duty/170
#define PWM_ADJ_MAX100	255				//100% = duty/255
#define PWM_ADJ_MAX075	(255*125/100)	// 75% = duty /318
#define PWM_ADJ_MAX050	(255*150/100)	// 50% = duty /382

class LmMotor
{
  public:
    LmMotor();
    void init();

    void onBldcString(String _msg);
    void onBldc(int _dir, int _duty);
    void offBldc();

  private:
    int defaultDuty;
	int outDuty;

};

//===  EXTERN VAR
extern unsigned long curr_ms_tick;

extern int gameStatus;
extern unsigned int	motTestAct;
extern unsigned int	motTestBbnIdx;

extern String STX_LM_MOTOR;

