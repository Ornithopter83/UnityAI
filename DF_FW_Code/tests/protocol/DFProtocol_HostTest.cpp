#include "../../libraries/DFProtocol/src/DFProtocol.h"

#include <assert.h>
#include <string.h>

int main()
{
    char buffer[128];
    unsigned char otaBuffer[DF_Protocol_RodOta_MaxFrameLength];
    unsigned char otaPayload[DF_Protocol_RodOta_MaxPayloadLength];
    DF_Protocol_MessageView message;
    DF_Protocol_RodOtaFrameView otaFrame;
    int length;
    unsigned int index;

    length = DF_Protocol_Encode(buffer, sizeof(buffer), DF_Protocol_MainToRod_AllOutputsOff, "00", 2);
    assert(length == 4);
    assert(strcmp(buffer, "2900") == 0);

    assert(DF_Protocol_Decode(buffer, (size_t)length, &message) == 0);
    assert(message.pid == DF_Protocol_Pid_MainToRod_AllOutputsOff);
    assert(message.payloadLength == 2);
    assert(DF_Protocol_Decode("0", 1, &message) < 0);
    assert(DF_Protocol_Decode("AA00", 4, &message) < 0);

    assert(DF_Protocol_RodOta_MaxFrameLength < 128);
    assert(DF_Protocol_RodOta_CalculateCrc32((const unsigned char*)"123456789", 9) == 0xcbf43926UL);
    assert((DF_Protocol_RodOta_StartFixedPayloadLength + DF_Protocol_RodOta_MaxVersionLength) <= DF_Protocol_RodOta_MaxPayloadLength);
    for (index = 0; index < DF_Protocol_RodOta_MaxPayloadLength; index++)
    {
        otaPayload[index] = (unsigned char)index;
    }
    length = DF_Protocol_RodOta_EncodeFrame(otaBuffer, sizeof(otaBuffer), DF_Protocol_RodOta_FrameData, 0x12345678UL, 0x01020304UL, otaPayload, sizeof(otaPayload));
    assert(length == DF_Protocol_RodOta_MaxFrameLength);
    assert(DF_Protocol_RodOta_DecodeFrame(otaBuffer, (size_t)length, &otaFrame) == 0);
    assert(otaFrame.frameType == DF_Protocol_RodOta_FrameData);
    assert(otaFrame.sessionId == 0x12345678UL);
    assert(otaFrame.sequence == 0x01020304UL);
    assert(otaFrame.payloadLength == DF_Protocol_RodOta_MaxPayloadLength);
    assert(memcmp(otaFrame.payload, otaPayload, sizeof(otaPayload)) == 0);

    otaBuffer[DF_Protocol_RodOta_HeaderLength + 5] ^= 0x80;
    assert(DF_Protocol_RodOta_DecodeFrame(otaBuffer, (size_t)length, &otaFrame) == -4);
    otaBuffer[DF_Protocol_RodOta_HeaderLength + 5] ^= 0x80;
    assert(DF_Protocol_RodOta_DecodeFrame(otaBuffer, (size_t)(length - 1), &otaFrame) == -2);
    assert(DF_Protocol_RodOta_EncodeFrame(otaBuffer, sizeof(otaBuffer), DF_Protocol_RodOta_FrameData, 1, 0, otaPayload, DF_Protocol_RodOta_MaxPayloadLength + 1) == -2);

    length = DF_Protocol_RodOta_EncodeFrame(otaBuffer, sizeof(otaBuffer), DF_Protocol_RodOta_FrameFinish, 7, 0, 0, 0);
    assert(length == (DF_Protocol_RodOta_HeaderLength + DF_Protocol_RodOta_CrcLength));
    assert(DF_Protocol_RodOta_DecodeFrame(otaBuffer, (size_t)length, &otaFrame) == 0);
    assert(otaFrame.payloadLength == 0);
    return 0;
}
