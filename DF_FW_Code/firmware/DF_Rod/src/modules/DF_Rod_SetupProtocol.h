#pragma once

#include <Arduino.h>

void uartPcHandler_NormalMode();
int targetAddrWrite(String msg);
void uartPcHandler_Setup();
void setRecvImuData();
int isRodNewBoard();
int setBoardType();
int getButtonType();
int getBreakType();
int getEncType();
int getBattType();
void settingIOtype();
void setSlaveMode();
int readSlaveMode();
int isSlaveSetupMode();
void Entry_SetupMode();
void Exit_SetupMode();
void ESP_Soft_Reset();
