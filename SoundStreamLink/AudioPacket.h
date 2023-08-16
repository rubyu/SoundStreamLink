#pragma once

#include <cstdint>
#include <Audioclient.h>

struct AudioPacket {
    // Audio stream format.
    const UINT32 SamplingRate;     // Sampling Frequency in Hz
    const UINT16 Channels;         // Number of channels 1=Mono 2=Sterio
    const UINT16 BitsPerSample;    // Number of bits per sample

    // Audio packet.
    UINT64 UpstreamDevicePosition;
    UINT32 Frames;
    UINT32 DataSize;
    BYTE* Data;

    AudioPacket(UINT32 samplingRate, UINT16 channels, UINT16 bitsPerSample) :
        SamplingRate(samplingRate),
        Channels(channels),
        BitsPerSample(bitsPerSample),
        UpstreamDevicePosition(0),
        Frames(0),
        DataSize(0),
        Data(nullptr)
    {}
};
