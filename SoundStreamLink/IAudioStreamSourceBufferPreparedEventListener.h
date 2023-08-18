#pragma once

#include <Audioclient.h>

class IAudioStreamSourceBufferPreparedEventListener {
public:
    virtual ~IAudioStreamSourceBufferPreparedEventListener() = default;
    virtual void AudioStreamSourceBufferPreparedCallback(UINT64 u64DevicePosition, BYTE* data, UINT32 numFrames) = 0;
};
