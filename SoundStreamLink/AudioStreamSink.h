#pragma once

#include <vector>
#include <set>
#include <cassert>
#include <iostream>
#include <Mmdeviceapi.h>
#include <functiondiscoverykeys.h>
#include "IUDPReceiverBufferPreparedEventListener.h"

class AudioStreamSink : public IUDPReceiverBufferPreparedEventListener {
private:
    std::vector<BYTE> buffer;

    size_t head;
    size_t head_upstream_sync_pos;
    size_t tail;
    size_t tail_upstream_sync_pos;
    size_t highest_upstream_sync_pos;

    // from WAVEFORMATEX
    size_t totalBufferCapacityInFrames; // Capacity of the buffer (in frames)
    size_t frameSize; // Size of one frame (in bytes)

    // for statistics
    size_t totalWrittenFrames;

    // buffer oparations
    bool NeedInitialization();

public:
    AudioStreamSink();
    void Write(UINT64 u64DevicePosition, BYTE* data, UINT32 numFrames);
    size_t Read(BYTE* output, UINT32 numFrames);
    void CommitRead(UINT32 numFrames);

    size_t GetTotalWrittenFrames() const;
    size_t GetSyncedFrames() const;

    void UDPReceiverBufferPreparedCallback(AudioPacket* packet);
};
