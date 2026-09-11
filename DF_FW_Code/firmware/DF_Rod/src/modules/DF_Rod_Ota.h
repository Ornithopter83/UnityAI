#pragma once

int DF_Rod_Ota_HandleFrame(const unsigned char *data, unsigned int length, const unsigned char *sourceAddress);
int DF_Rod_Ota_IsActive();
void DF_Rod_Ota_Process();

