#pragma once

#include <stddef.h>
#include <stdint.h>

enum
{
    DF_Protocol_MaxWireLength = 127,
    DF_Protocol_PidTextLength = 2,
    DF_Protocol_MacAddressTextLength = 17
};

typedef enum
{
    DF_Protocol_Pid_MainToRod_PowerOn = 5,
    DF_Protocol_Pid_MainToRod_RodInfoRequest = 7,
    DF_Protocol_Pid_MainToRod_Alive = 9,
    DF_Protocol_Pid_MainToRod_Break = 11,
    DF_Protocol_Pid_MainToRod_ImuDataControl = 13,
    DF_Protocol_Pid_MainToRod_VerticalMotor = 19,
    DF_Protocol_Pid_MainToRod_VersionRead = 21,
    DF_Protocol_Pid_MainToRod_ButtonLed = 27,
    DF_Protocol_Pid_MainToRod_AllOutputsOff = 29,
    DF_Protocol_Pid_MainToRod_MainAddress = 31,
    DF_Protocol_Pid_MainToRod_ApInfo = 33,
    DF_Protocol_Pid_MainToRod_Sleep = 34
} DF_Protocol_MainToRodPid;

typedef enum
{
    DF_Protocol_Pid_RodToMain_BoardType = 2,
    DF_Protocol_Pid_RodToMain_Alive = 10,
    DF_Protocol_Pid_RodToMain_ImuData = 14,
    DF_Protocol_Pid_RodToMain_Button = 16,
    DF_Protocol_Pid_RodToMain_Encoder = 18,
    DF_Protocol_Pid_RodToMain_Version = 22,
    DF_Protocol_Pid_RodToMain_ImuConnection = 24,
    DF_Protocol_Pid_RodToMain_Battery = 26,
    DF_Protocol_Pid_RodToMain_RodAddress = 32,
    DF_Protocol_Pid_RodToMain_Sleep = 34
} DF_Protocol_RodToMainPid;

extern const char DF_Protocol_MainToRod_PowerOn[];
extern const char DF_Protocol_MainToRod_RodInfoRequest[];
extern const char DF_Protocol_MainToRod_Alive[];
extern const char DF_Protocol_MainToRod_Break[];
extern const char DF_Protocol_MainToRod_ImuDataControl[];
extern const char DF_Protocol_MainToRod_VerticalMotor[];
extern const char DF_Protocol_MainToRod_VersionRead[];
extern const char DF_Protocol_MainToRod_ButtonLed[];
extern const char DF_Protocol_MainToRod_AllOutputsOff[];
extern const char DF_Protocol_MainToRod_MainAddress[];
extern const char DF_Protocol_MainToRod_ApInfo[];
extern const char DF_Protocol_MainToRod_Sleep[];

extern const char DF_Protocol_RodToMain_BoardType[];
extern const char DF_Protocol_RodToMain_Alive[];
extern const char DF_Protocol_RodToMain_ImuData[];
extern const char DF_Protocol_RodToMain_Button[];
extern const char DF_Protocol_RodToMain_Encoder[];
extern const char DF_Protocol_RodToMain_Version[];
extern const char DF_Protocol_RodToMain_ImuConnection[];
extern const char DF_Protocol_RodToMain_Battery[];
extern const char DF_Protocol_RodToMain_RodAddress[];
extern const char DF_Protocol_RodToMain_Sleep[];

typedef struct
{
    int pid;
    const char* payload;
    size_t payloadLength;
} DF_Protocol_MessageView;

enum
{
    DF_Protocol_RodOta_ProtocolVersion = 1,
    DF_Protocol_RodOta_HeaderLength = 15,
    DF_Protocol_RodOta_CrcLength = 4,
    DF_Protocol_RodOta_MaxPayloadLength = 96,
    DF_Protocol_RodOta_MaxFrameLength = 115,
    DF_Protocol_RodOta_DigestLength = 32,
    DF_Protocol_RodOta_MaxVersionLength = 24,
    DF_Protocol_RodOta_StartTargetOffset = 0,
    DF_Protocol_RodOta_StartImageSizeOffset = 1,
    DF_Protocol_RodOta_StartDigestOffset = 5,
    DF_Protocol_RodOta_StartVersionLengthOffset = 37,
    DF_Protocol_RodOta_StartFixedPayloadLength = 38,
    DF_Protocol_RodOta_StartVersionOffset = 38,
    DF_Protocol_RodOta_AckFrameTypeOffset = 0,
    DF_Protocol_RodOta_AckStatusOffset = 1,
    DF_Protocol_RodOta_AckNextSequenceOffset = 2,
    DF_Protocol_RodOta_AckBytesWrittenOffset = 6,
    DF_Protocol_RodOta_AckPayloadLength = 10,
    DF_Protocol_RodOta_ServiceImageLimit = 1250000
};

typedef enum
{
    DF_Protocol_RodOta_FrameStart = 1,
    DF_Protocol_RodOta_FrameData = 2,
    DF_Protocol_RodOta_FrameFinish = 3,
    DF_Protocol_RodOta_FrameAbort = 4,
    DF_Protocol_RodOta_FrameAck = 5
} DF_Protocol_RodOtaFrameType;

typedef enum
{
    DF_Protocol_RodOta_TargetRod = 1
} DF_Protocol_RodOtaTarget;

typedef enum
{
    DF_Protocol_RodOta_StatusOk = 0,
    DF_Protocol_RodOta_StatusInvalidFrame = 1,
    DF_Protocol_RodOta_StatusUnauthorized = 2,
    DF_Protocol_RodOta_StatusBusy = 3,
    DF_Protocol_RodOta_StatusSequenceError = 4,
    DF_Protocol_RodOta_StatusWriteError = 5,
    DF_Protocol_RodOta_StatusImageError = 6,
    DF_Protocol_RodOta_StatusSizeError = 7,
    DF_Protocol_RodOta_StatusTargetError = 8,
    DF_Protocol_RodOta_StatusCancelled = 9
} DF_Protocol_RodOtaStatus;

typedef struct
{
    unsigned char frameType;
    uint32_t sessionId;
    uint32_t sequence;
    const unsigned char* payload;
    unsigned int payloadLength;
} DF_Protocol_RodOtaFrameView;

int DF_Protocol_Encode(char* output, size_t outputCapacity, const char* pidText, const char* payload, size_t payloadLength);
int DF_Protocol_Decode(const char* input, size_t inputLength, DF_Protocol_MessageView* message);
uint32_t DF_Protocol_RodOta_CalculateCrc32(const unsigned char* data, size_t dataLength);
void DF_Protocol_RodOta_WriteUint32(unsigned char* output, uint32_t value);
uint32_t DF_Protocol_RodOta_ReadUint32(const unsigned char* input);
int DF_Protocol_RodOta_EncodeFrame(unsigned char* output, size_t outputCapacity, unsigned char frameType, uint32_t sessionId, uint32_t sequence, const unsigned char* payload, unsigned int payloadLength);
int DF_Protocol_RodOta_DecodeFrame(const unsigned char* input, size_t inputLength, DF_Protocol_RodOtaFrameView* frame);
