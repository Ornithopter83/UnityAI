//
// Potentiometer.h
//

#pragma once

#include "arduino.h"
#include "SPI.h"

#define RECV_MIN  0
#define RECV_MAX  255

#define TORQ_OFF_VALUE  0

// TORQ FULL / LIMIT SELECTION
#define TORQ_00W_FULL	0	// dummy

#define TORQ_10W_FULL	1
#define TORQ_20W_FULL	2
#define TORQ_30W_FULL	3

#define TORQ_10W_LIMIT	4
#define TORQ_20W_LIMIT	5
#define TORQ_30W_LIMIT	6

class Potentiometer
{
  public:
    Potentiometer();

    void init();
    void setValue(String _val);		// String
    void setValue(int _val);		// int
	void outValue(int _val);		// common Out
    void on(int val);		// DIR = NOT USE

	//void respTorqOut(String strVal);
	void respTorqOutVal(int val);

	int torqWattAdj = TORQ_30W_FULL;			// 3 = 30W_FULL
	//int torqWattAdj = TORQ_30W_LIMIT;	// 6 = 30W LIMIT
	
	int setAdjTorq = 0;

	int defaultVal = DEFAULT_TORQ; //60;		// Motor별 대기시 토크값 변경(10ㅉ-60, 30W-55, 40W - ? , 60W - ?)

  private:
    void digitalPortWrite(int _pinNum, int _val);
};


// EXTERN VAR
extern unsigned long curr_ms_tick;

extern String STX_TORQUE_MOTOR;

extern int gameStatus;
extern unsigned int	motTestAct;
extern unsigned int	motTestMainIdx;


