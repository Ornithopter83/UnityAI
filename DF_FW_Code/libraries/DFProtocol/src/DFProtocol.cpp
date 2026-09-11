#include "DFProtocol.h"

#include <string.h>

const char DF_Protocol_MainToRod_PowerOn[] = "05";
const char DF_Protocol_MainToRod_RodInfoRequest[] = "07";
const char DF_Protocol_MainToRod_Alive[] = "09";
const char DF_Protocol_MainToRod_Break[] = "11";
const char DF_Protocol_MainToRod_ImuDataControl[] = "13";
const char DF_Protocol_MainToRod_VerticalMotor[] = "19";
const char DF_Protocol_MainToRod_VersionRead[] = "21";
const char DF_Protocol_MainToRod_ButtonLed[] = "27";
const char DF_Protocol_MainToRod_AllOutputsOff[] = "29";
const char DF_Protocol_MainToRod_MainAddress[] = "31";
const char DF_Protocol_MainToRod_ApInfo[] = "33";
const char DF_Protocol_MainToRod_Sleep[] = "34";

const char DF_Protocol_RodToMain_BoardType[] = "02";
const char DF_Protocol_RodToMain_Alive[] = "10";
const char DF_Protocol_RodToMain_ImuData[] = "14";
const char DF_Protocol_RodToMain_Button[] = "16";
const char DF_Protocol_RodToMain_Encoder[] = "18";
const char DF_Protocol_RodToMain_Version[] = "22";
const char DF_Protocol_RodToMain_ImuConnection[] = "24";
const char DF_Protocol_RodToMain_Battery[] = "26";
const char DF_Protocol_RodToMain_RodAddress[] = "32";
const char DF_Protocol_RodToMain_Sleep[] = "34";

int DF_Protocol_Encode(char* output, size_t outputCapacity, const char* pidText, const char* payload, size_t payloadLength)
{
    size_t wireLength;

    if (output == 0 || pidText == 0 || payload == 0)
    {
        return -1;
    }
    if (pidText[0] < '0' || pidText[0] > '9' || pidText[1] < '0' || pidText[1] > '9' || pidText[2] != '\0')
    {
        return -2;
    }

    wireLength = DF_Protocol_PidTextLength + payloadLength;
    if (wireLength > DF_Protocol_MaxWireLength || outputCapacity <= wireLength)
    {
        return -3;
    }

    output[0] = pidText[0];
    output[1] = pidText[1];
    if (payloadLength > 0)
    {
        memcpy(output + DF_Protocol_PidTextLength, payload, payloadLength);
    }
    output[wireLength] = '\0';
    return (int)wireLength;
}

int DF_Protocol_Decode(const char* input, size_t inputLength, DF_Protocol_MessageView* message)
{
    if (input == 0 || message == 0)
    {
        return -1;
    }
    if (inputLength < DF_Protocol_PidTextLength || inputLength > DF_Protocol_MaxWireLength)
    {
        return -2;
    }
    if (input[0] < '0' || input[0] > '9' || input[1] < '0' || input[1] > '9')
    {
        return -3;
    }

    message->pid = ((int)(input[0] - '0') * 10) + (int)(input[1] - '0');
    message->payload = input + DF_Protocol_PidTextLength;
    message->payloadLength = inputLength - DF_Protocol_PidTextLength;
    return 0;
}

static int DF_Protocol_RodOta_IsFrameTypeValid(unsigned char frameType)
{
    return (DF_Protocol_RodOta_FrameStart <= frameType) && (DF_Protocol_RodOta_FrameAck >= frameType);
}

uint32_t DF_Protocol_RodOta_CalculateCrc32(const unsigned char* data, size_t dataLength)
{
    uint32_t crc = 0xffffffffUL;
    size_t dataIndex;
    unsigned int bitIndex;

    if ((0 == data) && (0 != dataLength))
    {
        return 0;
    }
    for (dataIndex = 0; dataIndex < dataLength; dataIndex++)
    {
        crc ^= (uint32_t)data[dataIndex];
        for (bitIndex = 0; bitIndex < 8; bitIndex++)
        {
            if (crc & 1UL)
            {
                crc = (crc >> 1) ^ 0xedb88320UL;
            }
            else
            {
                crc >>= 1;
            }
        }
    }
    return crc ^ 0xffffffffUL;
}

void DF_Protocol_RodOta_WriteUint32(unsigned char* output, uint32_t value)
{
    if (0 == output)
    {
        return;
    }
    output[0] = (unsigned char)((value >> 24) & 0xffUL);
    output[1] = (unsigned char)((value >> 16) & 0xffUL);
    output[2] = (unsigned char)((value >> 8) & 0xffUL);
    output[3] = (unsigned char)(value & 0xffUL);
}

uint32_t DF_Protocol_RodOta_ReadUint32(const unsigned char* input)
{
    if (0 == input)
    {
        return 0;
    }
    return ((uint32_t)input[0] << 24) |
           ((uint32_t)input[1] << 16) |
           ((uint32_t)input[2] << 8) |
           (uint32_t)input[3];
}

int DF_Protocol_RodOta_EncodeFrame(unsigned char* output, size_t outputCapacity, unsigned char frameType, uint32_t sessionId, uint32_t sequence, const unsigned char* payload, unsigned int payloadLength)
{
    size_t frameLength;
    uint32_t crc;

    if ((0 == output) || ((0 != payloadLength) && (0 == payload)))
    {
        return -1;
    }
    if ((!DF_Protocol_RodOta_IsFrameTypeValid(frameType)) || (DF_Protocol_RodOta_MaxPayloadLength < payloadLength))
    {
        return -2;
    }
    frameLength = DF_Protocol_RodOta_HeaderLength + payloadLength + DF_Protocol_RodOta_CrcLength;
    if (outputCapacity < frameLength)
    {
        return -3;
    }

    output[0] = 'D';
    output[1] = 'F';
    output[2] = 'R';
    output[3] = 'O';
    output[4] = DF_Protocol_RodOta_ProtocolVersion;
    output[5] = frameType;
    DF_Protocol_RodOta_WriteUint32(output + 6, sessionId);
    DF_Protocol_RodOta_WriteUint32(output + 10, sequence);
    output[14] = (unsigned char)payloadLength;
    if (0 != payloadLength)
    {
        memcpy(output + DF_Protocol_RodOta_HeaderLength, payload, payloadLength);
    }
    crc = DF_Protocol_RodOta_CalculateCrc32(output, DF_Protocol_RodOta_HeaderLength + payloadLength);
    DF_Protocol_RodOta_WriteUint32(output + DF_Protocol_RodOta_HeaderLength + payloadLength, crc);
    return (int)frameLength;
}

int DF_Protocol_RodOta_DecodeFrame(const unsigned char* input, size_t inputLength, DF_Protocol_RodOtaFrameView* frame)
{
    unsigned int payloadLength;
    size_t expectedLength;
    uint32_t expectedCrc;
    uint32_t receivedCrc;

    if ((0 == input) || (0 == frame))
    {
        return -1;
    }
    if ((DF_Protocol_RodOta_HeaderLength + DF_Protocol_RodOta_CrcLength) > inputLength || DF_Protocol_RodOta_MaxFrameLength < inputLength)
    {
        return -2;
    }
    if (('D' != input[0]) || ('F' != input[1]) || ('R' != input[2]) || ('O' != input[3]) ||
        (DF_Protocol_RodOta_ProtocolVersion != input[4]) || (!DF_Protocol_RodOta_IsFrameTypeValid(input[5])))
    {
        return -3;
    }
    payloadLength = input[14];
    if (DF_Protocol_RodOta_MaxPayloadLength < payloadLength)
    {
        return -2;
    }
    expectedLength = DF_Protocol_RodOta_HeaderLength + payloadLength + DF_Protocol_RodOta_CrcLength;
    if (inputLength != expectedLength)
    {
        return -2;
    }
    expectedCrc = DF_Protocol_RodOta_CalculateCrc32(input, DF_Protocol_RodOta_HeaderLength + payloadLength);
    receivedCrc = DF_Protocol_RodOta_ReadUint32(input + DF_Protocol_RodOta_HeaderLength + payloadLength);
    if (expectedCrc != receivedCrc)
    {
        return -4;
    }

    frame->frameType = input[5];
    frame->sessionId = DF_Protocol_RodOta_ReadUint32(input + 6);
    frame->sequence = DF_Protocol_RodOta_ReadUint32(input + 10);
    frame->payload = input + DF_Protocol_RodOta_HeaderLength;
    frame->payloadLength = payloadLength;
    return 0;
}
