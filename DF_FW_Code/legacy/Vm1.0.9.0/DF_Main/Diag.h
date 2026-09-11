//
// Diag.h
//

#ifndef _DIAG_h
#define _DIAG_h

#include "arduino.h"

/*
#define BOBBIN_CH 		PWM_CH0		// Channel )
#define BOBBIN_HZ 		22000		//22 Khz
#define BOBBIN_RESOL	10			// 10bit (0 ~1023)
*/

/*
class Diag
{
  public:
    Diag();
    void init();

    void entry();
    void exit();
	
    void inTest();
	void outTest();

	void inAllOff();
	void outAllOff();

	int diagCat;
  	int diagItem;
  	int diagAct;
  	int diagPara;

  private:
  	int diagSubMode;
};

*/

// Extern VAR
extern int mainMode;
extern int oldmainMode;
extern unsigned long curr_ms_tick;
//extern int ledControlMode;



// Extern Function

extern void diagInput_AllOff();
extern void diagOutput_AllOff();
extern void ledOn_MbBtm();
extern void initSet_LedContVal(int Act=1);
extern void motor_AllOff();
//extern void extLed_AllOff();
extern void extLed_controlOff();
extern void reelOut_AllOff();





#endif

