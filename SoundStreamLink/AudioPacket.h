#pragma once

#include <cstdint>
#include <Audioclient.h>

struct AudioPacket {
    //todo: timestamp

    // Audio stream format.
    UINT32 SamplingRate;     // Sampling Frequency in Hz
    UINT16 Channels;         // Number of channels 1=Mono 2=Sterio
    UINT16 BitsPerSample;    // Number of bits per sample

    // Audio packet.
    UINT64 UpstreamDevicePosition;
    UINT32 Frames;
    UINT32 DataSize;
    std::unique_ptr<BYTE[]> Data;

    AudioPacket() :
        SamplingRate(0),
        Channels(0),
        BitsPerSample(0),
        UpstreamDevicePosition(0),
        Frames(0),
        DataSize(0),
        Data(nullptr)
    {}
};
