#pragma once

#include <Audioclient.h>

class IBufferUpdateListener {
public:
    virtual ~IBufferUpdateListener() = default;
    virtual void bufferUpdated(UINT64 u64DevicePosition, BYTE* data, size_t numFrames) = 0;
};
