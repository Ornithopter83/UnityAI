#include "../DF_Main_Internal.h"
#include "DF_Main_RodOtaRelay.h"

#include <string.h>

static int DF_Main_RodOtaRelay_Active = 0;
static uint32_t DF_Main_RodOtaRelay_SessionId = 0;
static unsigned long DF_Main_RodOtaRelay_LastFrameAt = 0;

static int DF_Main_RodOtaRelay_HexValue(char value)
{
	if (('0' <= value) && ('9' >= value)) return value - '0';
	if (('A' <= value) && ('F' >= value)) return value - 'A' + 10;
	if (('a' <= value) && ('f' >= value)) return value - 'a' + 10;
	return -1;
}

static int DF_Main_RodOtaRelay_DecodeHex(const String& text, unsigned char *output, unsigned int capacity)
{
	unsigned int index;
	unsigned int byteCount;
	if (0 != (text.length() & 1)) return -1;
	byteCount = (unsigned int)text.length() / 2;
	if (capacity < byteCount) return -1;
	for (index = 0; index < byteCount; index++)
	{
		int highValue = DF_Main_RodOtaRelay_HexValue(text.charAt(index * 2));
		int lowValue = DF_Main_RodOtaRelay_HexValue(text.charAt((index * 2) + 1));
		if ((0 > highValue) || (0 > lowValue)) return -1;
		output[index] = (unsigned char)((highValue << 4) | lowValue);
	}
	return (int)byteCount;
}

static void DF_Main_RodOtaRelay_PrintHex(const unsigned char *data, unsigned int length)
{
	static const char digits[] = "0123456789ABCDEF";
	unsigned int index;
	if (!g_SerialEnable) return;
	Serial.print("$OR");
	for (index = 0; index < length; index++)
	{
		Serial.print(digits[(data[index] >> 4) & 0x0f]);
		Serial.print(digits[data[index] & 0x0f]);
	}
	Serial.println("%");
}

int DF_Main_RodOtaRelay_HandlePcCommand(const String& message)
{
	unsigned char frameBuffer[DF_Protocol_RodOta_MaxFrameLength];
	DF_Protocol_RodOtaFrameView frame;
	String hexText;
	int frameLength;

	if (!message.startsWith("$OR")) return 0;
	hexText = message.substring(3);
	if (hexText.endsWith("%")) hexText.remove(hexText.length() - 1);
	frameLength = DF_Main_RodOtaRelay_DecodeHex(hexText, frameBuffer, sizeof(frameBuffer));
	if ((0 >= frameLength) || (0 != DF_Protocol_RodOta_DecodeFrame(frameBuffer, frameLength, &frame)) ||
		(DF_Protocol_RodOta_FrameAck == frame.frameType))
	{
		if (g_SerialEnable) Serial.println("$OE1%");
		return 1;
	}
	if (DF_Protocol_RodOta_FrameStart == frame.frameType)
	{
		if (DF_Main_RodOtaRelay_Active && (DF_Main_RodOtaRelay_SessionId != frame.sessionId) &&
			(3000UL < (millis() - DF_Main_RodOtaRelay_LastFrameAt)))
		{
			DF_Main_RodOtaRelay_Active = 0;
			DF_Main_RodOtaRelay_SessionId = 0;
		}
		if ((DF_Main_RodOtaRelay_Active && (DF_Main_RodOtaRelay_SessionId != frame.sessionId)) ||
			((!DF_Main_RodOtaRelay_Active) && ((ROD_CONN != rod_conn_status) || rodRegistMode)))
		{
			if (g_SerialEnable) Serial.println("$OE3%");
			return 1;
		}
	}
	else if ((!DF_Main_RodOtaRelay_Active) || (DF_Main_RodOtaRelay_SessionId != frame.sessionId))
	{
		if (g_SerialEnable) Serial.println("$OE3%");
		return 1;
	}
	if (!eNow.writeRaw(frameBuffer, (unsigned int)frameLength))
	{
		if (g_SerialEnable) Serial.println("$OE5%");
	}
	else if (DF_Protocol_RodOta_FrameStart == frame.frameType)
	{
		DF_Main_RodOtaRelay_Active = 1;
		DF_Main_RodOtaRelay_SessionId = frame.sessionId;
	}
	DF_Main_RodOtaRelay_LastFrameAt = millis();
	return 1;
}

int DF_Main_RodOtaRelay_HandleRodFrame(const unsigned char *data, unsigned int length, const unsigned char *sourceAddress)
{
	DF_Protocol_RodOtaFrameView frame;
	if ((NULL == data) || (NULL == sourceAddress) || (19 > length)) return 0;
	if (('D' != data[0]) || ('F' != data[1]) || ('R' != data[2]) || ('O' != data[3])) return 0;
	if (0 != memcmp(sourceAddress, slave_board_addr, 6)) return 1;
	if ((0 != DF_Protocol_RodOta_DecodeFrame(data, length, &frame)) || (DF_Protocol_RodOta_FrameAck != frame.frameType)) return 1;
	if ((!DF_Main_RodOtaRelay_Active) || (DF_Main_RodOtaRelay_SessionId != frame.sessionId)) return 1;
	DF_Main_RodOtaRelay_PrintHex(data, length);
	if ((DF_Protocol_RodOta_AckPayloadLength == frame.payloadLength) &&
		(((DF_Protocol_RodOta_FrameFinish == frame.payload[DF_Protocol_RodOta_AckFrameTypeOffset]) &&
		  (DF_Protocol_RodOta_StatusOk == frame.payload[DF_Protocol_RodOta_AckStatusOffset])) ||
		 (DF_Protocol_RodOta_FrameAbort == frame.payload[DF_Protocol_RodOta_AckFrameTypeOffset])))
	{
		DF_Main_RodOtaRelay_Active = 0;
		DF_Main_RodOtaRelay_SessionId = 0;
		DF_Main_RodOtaRelay_LastFrameAt = 0;
	}
	return 1;
}
