#pragma once

#include <vector>
#include <set>
#include <cassert>
#include <iostream>
#include <Mmdeviceapi.h>
#include <functiondiscoverykeys.h>

class RingBuffer {
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

    void IncrementHead();
    void IncrementTail();
    void IncrementTailZerofill();
    bool IsInitialState();

public:
    RingBuffer(const WAVEFORMATEX& format, size_t numFrames);
    void Write(UINT64 u64DevicePosition, const BYTE* data, size_t numFrames);
    size_t Read(UINT64 u64DevicePosition, BYTE* output, size_t numFrames);
    size_t ReadAll(BYTE* output);

    size_t GetTotalWrittenFrames() const;
    size_t GetTotalZeroFilledFrames() const;
    size_t GetCurrentValidFrames() const;
    size_t GetCurrentZeroFilledFrames() const;
};
