#pragma once

#include <vector>
#include <set>
#include <cassert>
#include <iostream>
#include <Mmdeviceapi.h>
#include <functiondiscoverykeys.h>
#include "IAudioStreamSourceBufferPreparedEventListener.h"

class RingBuffer : public IAudioStreamSourceBufferPreparedEventListener {
private:
    std::vector<BYTE> buffer;
    size_t head;
    size_t head_upstream_sync_pos;
    size_t tail;
    size_t tail_upstream_sync_pos;

    // from WAVEFORMATEX
    size_t totalBufferCapacityInFrames; // Capacity of the buffer (in frames)
    size_t frameSize; // Size of one frame (in bytes)

    // for statistics
    size_t totalWrittenFrames;
    size_t totalZeroFilledFrames;
    size_t currentZeroFilledFrames;
    std::set<size_t> zeroFilledPositions;

    // buffer oparations
    void IncrementHead();
    void IncrementTail();
    void IncrementTailZerofill();
    bool IsInitialState();

public:
    RingBuffer(const WAVEFORMATEX& format, size_t numFrames);
    void Write(UINT64 u64DevicePosition, BYTE* data, UINT32 numFrames);
    size_t Read(UINT64 u64DevicePosition, BYTE* output, UINT32 numFrames);
    size_t ReadAll(BYTE* output);

    size_t GetTotalWrittenFrames() const;
    size_t GetTotalZeroFilledFrames() const;
    size_t GetCurrentValidFrames() const;
    size_t GetCurrentZeroFilledFrames() const;

    void AudioStreamSourceBufferPreparedCallback(UINT64 u64DevicePosition, BYTE* data, UINT32 numFrames);
};
