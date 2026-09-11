//
// Encoder.h
//

#ifndef _ENCODER_h
#define _ENCODER_h

#include "arduino.h"

//#define BTN_PIN   16
//#define BBN_ENC_A    17
//#define BBN_ENC_B    18

#define DIVISION_VAL	2
#define ZERO_VAL		0

#define MIN_COUNT    	0 	// 10000
#define MAX_COUNT    6000	//65000

class Encoder
{
  public:
    Encoder();

    void init();
    void rotate();

    void setRotateCallback(void (*rotateCallback)(int _dir, int _cnt));

	void setBbnCnt(unsigned int _cnt);
	void setLmCnt(unsigned int _cnt);

	int bbnDir = BBN_ENC_CW;
	//int encCnt = 0;
	unsigned int bbnCnt = (WIRE_HOME_BBN_CNT);			// 3 Meter = 3 * 100cm * 10 mm
	unsigned int lmCnt = 1;

  private:
    unsigned int count = (unsigned int)(WIRE_HOME_BBN_CNT);
	
    int last_ACLK;
    int last_BDT;
	int last_lmEnc;
    int current_ACLK;
    int current_BDT;
    int current_lmEnc;
    String oldStr;

    String fillZero(unsigned int _count);
    //String sendCount(int _dir, unsigned int _cnt);
};

// Extern VAR
extern unsigned long curr_ms_tick;


#endif
