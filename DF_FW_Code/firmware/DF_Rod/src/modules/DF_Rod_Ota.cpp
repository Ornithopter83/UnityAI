#include "../DF_Rod_Internal.h"
#include "DF_Rod_Ota.h"

#include <Update.h>
#include <mbedtls/sha256.h>
#include <string.h>

typedef struct
{
	int active;
	int hashInitialized;
	int restartPending;
	unsigned long restartAt;
	uint32_t sessionId;
	uint32_t nextSequence;
	uint32_t imageSize;
	uint32_t bytesWritten;
	unsigned char expectedDigest[DF_Protocol_RodOta_DigestLength];
	mbedtls_sha256_context hashContext;
} DF_Rod_OtaState;

static DF_Rod_OtaState DF_Rod_Ota_State;

static void DF_Rod_Ota_ResetState()
{
	if (DF_Rod_Ota_State.hashInitialized)
	{
		mbedtls_sha256_free(&DF_Rod_Ota_State.hashContext);
	}
	memset(&DF_Rod_Ota_State, 0, sizeof(DF_Rod_Ota_State));
}

static void DF_Rod_Ota_StopOutputs()
{
	rodVrtControl_Stop();
	rodBtnLedControl_Stop(0);
	rodBtnLedControl_Stop(1);
	digitalWrite(VRT_MOT_ON_PIN, VRT_MOT_OFF);
	digitalWrite(BTN_LED_LF_RED_PIN, BTN_LED_OFF);
	digitalWrite(BTN_LED_RT_BLUE_PIN, BTN_LED_OFF);
}

static void DF_Rod_Ota_SendAck(const DF_Protocol_RodOtaFrameView *request, unsigned char status)
{
	unsigned char payload[DF_Protocol_RodOta_AckPayloadLength];
	unsigned char frame[DF_Protocol_RodOta_MaxFrameLength];
	int frameLength;
	payload[DF_Protocol_RodOta_AckFrameTypeOffset] = request->frameType;
	payload[DF_Protocol_RodOta_AckStatusOffset] = status;
	DF_Protocol_RodOta_WriteUint32(payload + DF_Protocol_RodOta_AckNextSequenceOffset, DF_Rod_Ota_State.nextSequence);
	DF_Protocol_RodOta_WriteUint32(payload + DF_Protocol_RodOta_AckBytesWrittenOffset, DF_Rod_Ota_State.bytesWritten);
	frameLength = DF_Protocol_RodOta_EncodeFrame(frame, sizeof(frame), DF_Protocol_RodOta_FrameAck,
		request->sessionId, request->sequence, payload, sizeof(payload));
	if (0 < frameLength)
	{
		eNow.writeRaw(frame, (unsigned int)frameLength);
	}
}

static unsigned char DF_Rod_Ota_Start(const DF_Protocol_RodOtaFrameView *frame)
{
	unsigned int versionLength;
	uint32_t imageSize;
	if ((0 == frame->sessionId) || (DF_Protocol_RodOta_StartFixedPayloadLength > frame->payloadLength)) return DF_Protocol_RodOta_StatusInvalidFrame;
	if (DF_Protocol_RodOta_TargetRod != frame->payload[DF_Protocol_RodOta_StartTargetOffset]) return DF_Protocol_RodOta_StatusTargetError;
	versionLength = frame->payload[DF_Protocol_RodOta_StartVersionLengthOffset];
	if ((0 == versionLength) || (DF_Protocol_RodOta_MaxVersionLength < versionLength) ||
		(frame->payloadLength != (DF_Protocol_RodOta_StartFixedPayloadLength + versionLength))) return DF_Protocol_RodOta_StatusInvalidFrame;
	imageSize = DF_Protocol_RodOta_ReadUint32(frame->payload + DF_Protocol_RodOta_StartImageSizeOffset);
	if ((0 == imageSize) || (DF_Protocol_RodOta_ServiceImageLimit < imageSize)) return DF_Protocol_RodOta_StatusSizeError;
	if (DF_Rod_Ota_State.active)
	{
		if ((DF_Rod_Ota_State.sessionId == frame->sessionId) && (DF_Rod_Ota_State.imageSize == imageSize) &&
			(0 == memcmp(DF_Rod_Ota_State.expectedDigest, frame->payload + DF_Protocol_RodOta_StartDigestOffset, DF_Protocol_RodOta_DigestLength)))
		{
			return DF_Protocol_RodOta_StatusOk;
		}
		return DF_Protocol_RodOta_StatusBusy;
	}
	DF_Rod_Ota_ResetState();
	if (!Update.begin(imageSize, U_FLASH)) return DF_Protocol_RodOta_StatusWriteError;
	DF_Rod_Ota_State.sessionId = frame->sessionId;
	DF_Rod_Ota_State.imageSize = imageSize;
	memcpy(DF_Rod_Ota_State.expectedDigest, frame->payload + DF_Protocol_RodOta_StartDigestOffset, DF_Protocol_RodOta_DigestLength);
	mbedtls_sha256_init(&DF_Rod_Ota_State.hashContext);
	DF_Rod_Ota_State.hashInitialized = 1;
	if (0 != mbedtls_sha256_starts_ret(&DF_Rod_Ota_State.hashContext, 0))
	{
		Update.abort();
		DF_Rod_Ota_ResetState();
		return DF_Protocol_RodOta_StatusImageError;
	}
	DF_Rod_Ota_State.active = 1;
	DF_Rod_Ota_StopOutputs();
	return DF_Protocol_RodOta_StatusOk;
}

static unsigned char DF_Rod_Ota_Data(const DF_Protocol_RodOtaFrameView *frame)
{
	if ((!DF_Rod_Ota_State.active) || (DF_Rod_Ota_State.sessionId != frame->sessionId)) return DF_Protocol_RodOta_StatusInvalidFrame;
	if (frame->sequence < DF_Rod_Ota_State.nextSequence) return DF_Protocol_RodOta_StatusOk;
	if (frame->sequence != DF_Rod_Ota_State.nextSequence) return DF_Protocol_RodOta_StatusSequenceError;
	if ((0 == frame->payloadLength) || ((DF_Rod_Ota_State.bytesWritten + frame->payloadLength) > DF_Rod_Ota_State.imageSize)) return DF_Protocol_RodOta_StatusSizeError;
	if (frame->payloadLength != Update.write((uint8_t *)frame->payload, frame->payloadLength)) return DF_Protocol_RodOta_StatusWriteError;
	if (0 != mbedtls_sha256_update_ret(&DF_Rod_Ota_State.hashContext, frame->payload, frame->payloadLength)) return DF_Protocol_RodOta_StatusImageError;
	DF_Rod_Ota_State.bytesWritten += frame->payloadLength;
	DF_Rod_Ota_State.nextSequence++;
	return DF_Protocol_RodOta_StatusOk;
}

static unsigned char DF_Rod_Ota_Finish(const DF_Protocol_RodOtaFrameView *frame)
{
	unsigned char actualDigest[DF_Protocol_RodOta_DigestLength];
	if (DF_Rod_Ota_State.restartPending && (DF_Rod_Ota_State.sessionId == frame->sessionId) &&
		(DF_Rod_Ota_State.nextSequence == frame->sequence)) return DF_Protocol_RodOta_StatusOk;
	if ((!DF_Rod_Ota_State.active) || (DF_Rod_Ota_State.sessionId != frame->sessionId)) return DF_Protocol_RodOta_StatusInvalidFrame;
	if ((frame->sequence != DF_Rod_Ota_State.nextSequence) || (0 != frame->payloadLength)) return DF_Protocol_RodOta_StatusSequenceError;
	if (DF_Rod_Ota_State.bytesWritten != DF_Rod_Ota_State.imageSize) return DF_Protocol_RodOta_StatusSizeError;
	if (0 != mbedtls_sha256_finish_ret(&DF_Rod_Ota_State.hashContext, actualDigest)) return DF_Protocol_RodOta_StatusImageError;
	mbedtls_sha256_free(&DF_Rod_Ota_State.hashContext);
	DF_Rod_Ota_State.hashInitialized = 0;
	if (0 != memcmp(actualDigest, DF_Rod_Ota_State.expectedDigest, sizeof(actualDigest)))
	{
		Update.abort();
		DF_Rod_Ota_ResetState();
		return DF_Protocol_RodOta_StatusImageError;
	}
	if (!Update.end(false))
	{
		DF_Rod_Ota_ResetState();
		return DF_Protocol_RodOta_StatusWriteError;
	}
	DF_Rod_Ota_State.restartPending = 1;
	DF_Rod_Ota_State.restartAt = millis() + 500UL;
	return DF_Protocol_RodOta_StatusOk;
}

int DF_Rod_Ota_HandleFrame(const unsigned char *data, unsigned int length, const unsigned char *sourceAddress)
{
	DF_Protocol_RodOtaFrameView frame;
	unsigned char status;
	if ((NULL == data) || (NULL == sourceAddress) || (19 > length)) return 0;
	if (('D' != data[0]) || ('F' != data[1]) || ('R' != data[2]) || ('O' != data[3])) return 0;
	if (0 != memcmp(sourceAddress, main_board_addr, 6)) return 1;
	if (0 != DF_Protocol_RodOta_DecodeFrame(data, length, &frame)) return 1;
	if (DF_Protocol_RodOta_FrameStart == frame.frameType) status = DF_Rod_Ota_Start(&frame);
	else if (DF_Protocol_RodOta_FrameData == frame.frameType) status = DF_Rod_Ota_Data(&frame);
	else if (DF_Protocol_RodOta_FrameFinish == frame.frameType) status = DF_Rod_Ota_Finish(&frame);
	else if (DF_Protocol_RodOta_FrameAbort == frame.frameType)
	{
		if (DF_Rod_Ota_State.restartPending)
		{
			status = DF_Protocol_RodOta_StatusBusy;
		}
		else
		{
			if (Update.isRunning()) Update.abort();
			DF_Rod_Ota_ResetState();
			status = DF_Protocol_RodOta_StatusCancelled;
		}
	}
	else status = DF_Protocol_RodOta_StatusInvalidFrame;
	DF_Rod_Ota_SendAck(&frame, status);
	return 1;
}

int DF_Rod_Ota_IsActive()
{
	return DF_Rod_Ota_State.active;
}

void DF_Rod_Ota_Process()
{
	if (DF_Rod_Ota_State.restartPending && ((long)(millis() - DF_Rod_Ota_State.restartAt) >= 0))
	{
		ESP.restart();
	}
}
