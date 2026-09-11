#pragma once

#include <Arduino.h>
#include <esp_now.h>

void rotateChangeCallback(int _dir, int _cnt);
void sent_cb_esp_now_sts(const uint8_t* mac_addr, esp_now_send_status_t status);
void recv_cb_esp_now_msg(const uint8_t *mac_info, const uint8_t *data, int data_len);
int DF_Main_Communication_TakeReceived(char *data, unsigned int capacity, unsigned int *length, uint8_t *sourceAddress, unsigned long *overwrittenCount, unsigned long *invalidCount);
void DF_Main_Communication_ProcessSendResult();
void battLevelSet( String msg);
void setRod_Connected();
void uartRecvHandlerImp();
bool isParaCheckOk(int idx, int val);
void waveType_TorqSet();
int HitType_TorqSet(int _fishKind);
int isExecBldc();
void torqMot_OffControlStart(unsigned int onTime);
void torqMot_OffControlStop();
void torqMotor_OffTime_Control();
void bbnMotor_OffTime_Start(unsigned int time);
void bbnMotor_OffTime_Control();
void lmMotor_OffTime_Start(unsigned int time);
void lmMotor_OffTime_Control();
void clear_ExecFlag();
void execGameOver();
void setVal_Hit_MainPwr(String msg);
void SetIMU_Measure_Out_OnOff(unsigned int _setClr);
void sendSleepEnable(int Mode, int Time, int Id);
void sendCurrentStatus();

extern unsigned int bbnMotorStop_Step;
extern unsigned int gBbnHallCount;
