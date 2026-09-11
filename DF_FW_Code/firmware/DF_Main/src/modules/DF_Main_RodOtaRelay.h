#pragma once

#include <Arduino.h>

int DF_Main_RodOtaRelay_HandlePcCommand(const String& message);
int DF_Main_RodOtaRelay_HandleRodFrame(const unsigned char *data, unsigned int length, const unsigned char *sourceAddress);

