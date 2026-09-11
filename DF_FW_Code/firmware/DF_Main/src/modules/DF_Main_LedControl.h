#pragma once

#include <Arduino.h>

void initSet_LedContVal(int Act);
void ledCont_OffStart(unsigned int pos);
void ledCont_OnStart(unsigned int pos);
void ledCont_BlinkStart(unsigned int pos);
void execledCont_Blink(int pos);
void ledCont_DimmStart(unsigned int pos);
void execledCont_Dimm(int pos);
void old_execCentLed_Dimming_Control(int color, int sec);
void extLed_Off(unsigned int idx);
void extLed_On(unsigned int idx);
void extLed_Control();

extern unsigned int ext_led_dim_step;
