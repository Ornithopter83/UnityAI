#pragma once

#include <Arduino.h>

void anaDeviceContReq(String msg);
void anaRestDistance(String msg);
void ana_AngleControl( String msg );
void ana_MainMotControl( String msg );
void ana_BobbinControl_Check(String msg);
void ana_LineMotControl_Check(String msg);
void ana_BobbinControl(String msg);
void ana_LmMotControl(String msg);
void lmHome_Control_Start(int _init, int tmReqDuty);
void lmHome_Control_Stop();
void lmLeft_Control_Start(int fish, int pwr, int ReqTm);
void lmLeft_Control_Return();
void lmLeft_Control_Stop();
void lmRight_Control_Start(int fish, int pwr, int ReqTm);
void lmRight_Control_Return();
void lmRight_Control_Stop();
void lmCenter_Control_Start(int fish, int pwr);
void lmCenter_Control_Stop();
void lmReturn_Control_Start(int pwr);
void lmReturn_Control_Stop();
void lmHoldOn_Stop();
void lmHome_Control_2();
void lmHome_Control();
void lmPosition_Set(int posi);
void lmLeft_Control();
void lmRight_Control();
void lmReturn_Control();
void anaMotOutRateSet(String msg);
void checkRodTimer();
void set_rod_conn_status(unsigned int sts);
void condition_fw_check_10ms();
