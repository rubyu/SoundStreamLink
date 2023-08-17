#pragma once

#include <Audioclient.h>

class IBufferUpdateListener {
public:
    virtual ~IBufferUpdateListener() = default;
    virtual void bufferUpdateCallback(UINT64 u64DevicePosition, BYTE* data, UINT32 numFrames) = 0;
};
