// Encoder.h

#pragma once

#include "arduino.h"

#define DIVISION_VAL  2
#define MIN_COUNT   -40
#define MAX_COUNT   40

#define ENC_MUL_V2	8	// 10	// 출력 카운터 수
#define ENC_DIV_V2	12			// 디바이스 카운터 수 ( 12펄스/회전 )
#define MAX_COUNT_V2  ((MAX_COUNT * ENC_DIV_V2 ) / ENC_MUL_V2) // 48	// = 40 * 12/10

// (Vr1.0.1.0) 엔코더 계산 방식 변경
#define ENC_MUL_V3	8	// 10	// 출력 카운터 수
#define ENC_DIV_V3	6			// 디바이스 카운터 수 ( 12펄스/회전 )
#define MAX_COUNT_V3  ((MAX_COUNT * ENC_DIV_V3 ) / ENC_MUL_V3) // 48	// = 40 * 12/10
#define ENC_MUL_VX		ENC_MUL_V3
#define ENC_DIV_VX		ENC_DIV_V3
#define MAX_COUNT_VX	MAX_COUNT_V3

class Encoder
{
  public:
    Encoder();

    void init();
    void rotate();

    void setRotateCallback(void (*rotateCallback)(String));
	String fillZero2char(int _count);
	int count = 0;

  private:
    int lastCLK;
    int lastDT;
    int currentCLK;
    int currentDT;
    String oldStr;

    String sendCount();
};


extern unsigned long curr_ms_tick;
//extern int isRodNewBoard();
extern int getEncType();

