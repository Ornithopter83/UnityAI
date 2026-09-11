//
// Led.h
//

#ifndef _LED_h
#define _LED_h

#include "arduino.h"

//#define BTN_PIN   16
//#define BBN_ENC_A    17
//#define BBN_ENC_B    18


//== DEFINE MAIN_LED Config
#define CENT_HZ 		22000		//9 Khz
#define OTHER_HZ 		22000		//8.5 Khz
#define LED_RESOL_8		8			// 8bit (0 ~1023)
#define LED_RESOL_10		10			// 10bit (0 ~1023)


// Define MAIN LED POSITION
//#define LED_CENTER		0
//#define LED_BOTTOM		1
//#define LED_OPT_LEFT	2
//#define LED_OPT_RIGHT	3

//macro
#if LED_PWM_INVERT
#define mapled(a) map(a, 255,0, 0,1023)		// 반전 : 0~ 255를 1023 ~ 0
#else
#define mapled(a) map(a, 0,255, 0,1023)		// 정상 : 0 ~ 255를 0~1023
#endif

// macro LedOut is AnalogWrite = PWM OUTPUT(1Khz), 분해능 8bit(0~255)
//#define	ledoutan	alogWrite
//#define ledOut(a,b)	analogWrite(a,(b*ledVal.light/100))
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

#endif

