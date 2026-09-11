#pragma once

#include <Arduino.h>
#include "../Diag.h"

void diagInput_AllOff();
void diagInput_BoardInAllOff();
void diagInput_BoardInAllOn();
void DiagInput_MotInAllOff();
void DiagInput_MotInAllOn();
void DiagInput_MainEcoderInAllOn();
void DiagInput_MechaInAllOff();
void DiagInput_ElecInAllOff();
void DiagInput_ElecInAllOn();
void diagInput_Cat0(String _msg);
void diagIput_Cat1(String _msg);
void diagInput_Cat3(String _msg);
void diagInput_Cat5(String _msg);
void diagInputTest(String _msg);
void diagOutput_AllOff();
void diagOutput_BoardOutAllOff();
void setRGBcolor(int pos, int Color);
void setOutRGBcolor(int pos, int Color);
void diagLmMotor_TimeoutStart(unsigned int _time);
void diagLmMotor_TimeOutStop();
void diagLmMotor_TimeoutControl();
void diagBbnMotor_TimeoutStart(unsigned int _time);
void diagBbnMotor_TimeoutControl();
void daigBbnMotor_TokTokEnd();
void diagBbnMotor_TokTokStart(unsigned int _ackKind, unsigned int _cnt);
void diagBbnMotor_TokTokControl(unsigned _opKind);
void diagOut_Cat1(String _msg);
void diagOut_Cat3(String _msg);
void diagOutputTest(String _msg);
void findDiagCode(String msg, stDiagCode code);
int controlDiagInput(String msg);
int controlDiagOutput(String msg);
void diagOutAllOff();
void Set_WaveData( String msg);
void Set_BiteData( String msg);
void Set_HitData(String msg);
void Set_BreakData( String msg);
void Set_ExtledData(String msg);
void Set_BbnTokTokData( String msg);
void Set_TorqAdjValData( String msg);
void Set_LedValue( String msg);
void Set_TorqHitHoldOn( String msg);
void Set_EnbData( String msg);
void subAcOff_WithOtherOff();
int CheckAppCmd();
int CheckPowerSwitch();
void check_PowerSwitch_Change_10ms();

extern int diagOutflag_lmMotTimeOut;
