#pragma once

#include <cstdint>
#include <Audioclient.h>

struct AudioPacket {
    // Audio stream format.
    UINT32 SamplingRate;     // Sampling Frequency in Hz
    UINT16 Channels;         // Number of channels 1=Mono 2=Sterio
    UINT16 BitsPerSample;    // Number of bits per sample

    // Audio packet.
    UINT64 UpstreamDevicePosition;
    bool AUDCLNT_BUFFERFLAGS_SILENT;
    bool AUDCLNT_BUFFERFLAGS_DATA_DISCONTINUITY;
    bool AUDCLNT_BUFFERFLAGS_TIMESTAMP_ERROR;
    UINT32 Frames;
    UINT32 DataSize;
    BYTE* Data;
};
