#include "AudioStreamSink.h"
#include "Utility.h"

bool AudioStreamSink::NeedInitialization() {
    return 0 == head && 0 == tail;
}

AudioStreamSink::AudioStreamSink()
    : samplingRate(0),
    channels(0),
    bitsPerSample(0), 
    head(0),
    head_upstream_sync_pos(0),
    tail(0),
    tail_upstream_sync_pos(0),
    highest_upstream_sync_pos(0),
    totalBufferCapacityInFrames(0),
    frameSize(0),
    totalWrittenFrames(0) {}

UINT32 AudioStreamSink::GetSamplingRate() {
    return samplingRate;
}

UINT16 AudioStreamSink::GetChannels() {
    return channels;
}

UINT16 AudioStreamSink::GetBitsPerSample() {
    return bitsPerSample;
}

void AudioStreamSink::Write(UINT64 u64DevicePosition, BYTE* data, UINT32 numFrames) {
    if (0 == numFrames) return;
    assert(0 <= u64DevicePosition);
    
    if (NeedInitialization()) {
        head_upstream_sync_pos = u64DevicePosition;
        tail = totalBufferCapacityInFrames - 1;
        tail_upstream_sync_pos = head_upstream_sync_pos + totalBufferCapacityInFrames - 1;
    }
    assert(head != tail);
    size_t upstream_pos = u64DevicePosition;
    size_t buffer_pos;

    size_t overflow = tail_upstream_sync_pos - upstream_pos;
    if (0 < overflow) {
        std::cout << "fill the gap with zero: " << overflow << " frames" << std::endl;
        buffer_pos = tail + 1;
        for (size_t i = 0; i < overflow; i++) {
            auto target = buffer.begin() + buffer_pos * frameSize;
            std::fill(target, target, 0);
            buffer_pos = (buffer_pos + 1) % totalBufferCapacityInFrames;
        }
    }

    for (size_t i = 0; i < numFrames; i++) {
        if (upstream_pos < head_upstream_sync_pos) {
            std::cout << "out of range: upstream_pos < head_upstream_sync_pos" << std::endl;
            std::cout << "upstream_pos: " << upstream_pos << std::endl;
            upstream_pos++;
            continue;
        }

        buffer_pos = (upstream_pos - head_upstream_sync_pos) % totalBufferCapacityInFrames;
        //std::cout << "buffer_pos: " << buffer_pos << std::endl;

        std::memcpy(&buffer[buffer_pos * frameSize], data + i * frameSize, frameSize);
        totalWrittenFrames++;
        upstream_pos++;
    }
    
    if (highest_upstream_sync_pos < upstream_pos) {
        std::cout << "highest_upstream_sync_pos: " << highest_upstream_sync_pos << std::endl;
        highest_upstream_sync_pos = upstream_pos;
    }

    if (tail_upstream_sync_pos < upstream_pos) {
        overflow = tail_upstream_sync_pos - upstream_pos;
        std::cout << "overwrite: " << overflow << " frames" << std::endl;
        head = (head + overflow) % totalBufferCapacityInFrames;
        head_upstream_sync_pos += overflow;
        tail = (tail + overflow) % totalBufferCapacityInFrames;
        tail_upstream_sync_pos += overflow;
    }

    std::cout << "head: " << head << std::endl;
    std::cout << "head_upstream_sync_pos: " << head_upstream_sync_pos << std::endl;
    std::cout << "tail: " << tail << std::endl;
    std::cout << "tail_upstream_sync_pos: " << tail_upstream_sync_pos << std::endl;
    std::cout << "highest_upstream_sync_pos: " << highest_upstream_sync_pos << std::endl;
}

size_t AudioStreamSink::Read(BYTE* output, UINT32 numFrames) {
    assert(head != tail);
    assert(numFrames <= totalBufferCapacityInFrames);
    size_t buffer_pos = head;
    for (size_t i = 0; i < numFrames; i++) {
        std::memcpy(output + i * frameSize, &buffer[buffer_pos * frameSize], frameSize);
        buffer_pos = (buffer_pos + 1) % totalBufferCapacityInFrames;
    }
    return numFrames;
}

void AudioStreamSink::CommitRead(UINT32 numFrames) {
    size_t buffer_pos = head;
    for (size_t i = 0; i < numFrames; i++) {
        auto target = buffer.begin() + buffer_pos * frameSize;
        std::fill(target, target, 0);
        buffer_pos = (buffer_pos + 1) % totalBufferCapacityInFrames;
    }
    head = (head + numFrames) % totalBufferCapacityInFrames;
    head_upstream_sync_pos += numFrames;
    tail = (tail + numFrames) % totalBufferCapacityInFrames;
    tail_upstream_sync_pos += numFrames;
}

size_t AudioStreamSink::GetTotalWrittenFrames() const {
    return totalWrittenFrames;
}

size_t AudioStreamSink::GetSyncedFrames() const {
    return highest_upstream_sync_pos - head_upstream_sync_pos;
}

void AudioStreamSink::UDPReceiverBufferPreparedCallback(AudioPacket* packet) {
    if (0 == samplingRate) {
        samplingRate = packet->SamplingRate;
        channels = packet->Channels;
        bitsPerSample = packet->BitsPerSample;

        size_t nBlockAlign = packet->Channels * packet->BitsPerSample / 8;
        size_t frames = CalculateFramesForDurationMilliseconds(packet->SamplingRate, packet->Channels, 5000);

        totalBufferCapacityInFrames = frames;
        frameSize = nBlockAlign;

        buffer.resize(frames * nBlockAlign);
    }
    Write(packet->UpstreamDevicePosition, packet->Data.get(), packet->Frames);
}
