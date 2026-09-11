#pragma once

#include <Arduino.h>

void ana_TestExecution(String msg);
void analedContCmd(String msg);
void anaRanding(String msg);
void anaFastHold(String msg);
void anaFastComboBtn(String msg);
void sendRodVrtCmd(int act, unsigned int cnt, unsigned int ontime, unsigned int offtime);
void sendRodBtnLedCmd(int posi, int act, unsigned int cnt, unsigned int ontime, unsigned int offtime);
short lmFish_3Level_Set(short pwr);
void lmReturnOrHome_Start();
void wireHomeControl_Start();
void wireHomeControl_Stop();
void wireHome_Control();
void setWireHome_EncCnt();
void anaGameStatusWAT(String msg);
void stbyTorOffStop_Check();
void anaGameStatus(String msg);
void anaDeviceInfoReq(String msg);
void anaGameEnbOver(String msg);
void apAlive_Check();
void send2RodAlive_CntCheck();
void rodRegistToStop();
void rodRegistToStart();
void rodRegi3secToStart();
void rodRegistToControl();
void rodRegistCancel();
void anaRodRegist(String msg);
void rodRegistExec(String msg);
void tmInput_BoardElecMonitorExec(short _exec);
void tmInput_BbnMotorMonitorExec(short _exec);
void anaTmInputTestReq(String msg);
void anaTmOutputTestReq(String msg);
void anaApInfo(String msg);
void anaMainAlive(String msg);

extern unsigned int cnt;
extern unsigned int reqCnt;
extern byte nvm_fish_level;
