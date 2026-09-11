//
// Led.h
//

#pragma once

#include "arduino.h"



//== DEFINE MAIN_LED Config
#define CENT_HZ 		22000		//9 Khz
#define OTHER_HZ 		22000		//8.5 Khz
#define LED_RESOL_8		8			// 8bit (0 ~1023)
#define LED_RESOL_10		10			// 10bit (0 ~1023)


// Define MAIN LED POSITION

//macro
#define mapled(a) map(a, 0,255, 0,1023)		// 정상 : 0 ~ 255를 0~1023

// macro LedOut is AnalogWrite = PWM OUTPUT(1Khz), 분해능 8bit(0~255)
#define ledOut(a,b)	analogWrite(a,(b*ledLight/100))		// light 1~100 [%]

// led Position
enum _ledPos
{
	LEDPOS_CENT_R = 0,
	LEDPOS_CENT_G,
	LEDPOS_CENT_B,
	LEDPOS_CENT_RGB,	// 3
	LEDPOS_BTM,			// 4
	LEDPOS_LEFT,		// 5
	LEDPOS_RIGHT,		// 6

	LEDPOS_MAX,			// 7
};

class Led
{
  public:
    Led();

    void init();
    void on(unsigned int ledpos, int _pwm);
    void RGBon(unsigned int ledpos, int Rval, int Gval, int Bval);
	void off(unsigned int ledpos);
	void blink(unsigned int ledpos, int btime, int bCnt);

	String fillZero(int _count);

    void LedCallback(void (*ledCallback)(String));	

  private:
  /*
    int count;
    int last_ACLK;
    int last_BDT;
    int current_ACLK;
    int current_BDT;
    String oldStr;

    String fillZero(int _count);
    String ledCount();
 */
    int blinkCnt;
  
};


// Extern VAR
extern unsigned long curr_ms_tick;
//extern stLedVal ledVal;
extern unsigned int ledLight;
