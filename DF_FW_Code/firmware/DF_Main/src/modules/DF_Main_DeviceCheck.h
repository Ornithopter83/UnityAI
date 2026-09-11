#pragma once

#include <Arduino.h>

void devChkReady_Control();
void resp_MainBoard_Ver();
void devchk_OkEnd();
void send2resp_FwSelfControlCmd(String Cmd, int torq);
String stringTo3char(int val);
void devChkMsgSend(String msg);
void devchk_bbnAllStart();
void devchk_lmMotStart();
void devCheckFlagStep_AllClear();
void devchk_bbnAll_Control();
void devchk_lmMot_Control();
void devchk_lmEnc_Control();
void devReadyCheck();
void send_Error_Clr();
