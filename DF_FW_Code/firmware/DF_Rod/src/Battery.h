#pragma once

#include "arduino.h"


#define BATTERY_PER_MIN_0		0
#define BATTERY_PER_MIN_1		1
#define BATTERY_PER_LOW		20
#define BATTERY_PER_MAX		99

//-- OLD ROD, 3350mA
#define BATTERY_MIN_VALUE  2795		// 9.0 V = 0%
#define BATTERY_MAX_VALUE  4095		// 12.0 V = 100%

//-- NEW ROD, 800mA
#define BATTERY_MIN_VALUE_NEW  	2828	// 기존 20%를 0%로 설정함(Vr1.0.1.0)
#define BATTERY_MAX_VALUE_NEW   3546	// 기존 82%를 100%로 설정함(Vr1.0.1.0)

#define BAT_BUF_SIZE 		20		// AVG 20 SEC ( 1 sec * 20 Times) 

#define	BAT_UPDATE_INTVL	(60*1000)		// 60sec
#define	BAT_UPDATE_GAP		2				// 

class Battery
{
  public:

    Battery();
//    void init();
	void init();
    //void scanAvg();
    void scanAvgInteg();
    void scanKalFilter();

    String getLevel3Char(int _lvl);
    bool isLowLevel();

	int ad_buff[BAT_BUF_SIZE];
	int ad_idx = 0;

	int adc_value;						// Vr1.0.1.0
	long sum_ad;
	int avg_ad;
	int integ_ad;

  	int rest_lvl = -2;
	int old_restLvl = -2;		// PowerOn 1st Change
	int	old_dir = 0;					// Vr1.0.1.0
	unsigned long last_tick = 9999999;	// *
	int	chk_cnt = 2;					// *

	String restLvlStr;
	
  private:

    bool lowState = false;

};

// Extern Define
//extern int isRodNewBoard();
extern int getBattType();

