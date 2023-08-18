#include "AudioStreamSink.h"
#include "Utility.h"

bool AudioStreamSink::NeedInitialization() {
    return 0 == head && 0 == tail;
}

AudioStreamSink::AudioStreamSink()
    : head(0),
    head_upstream_sync_pos(0),
    tail(0),
    tail_upstream_sync_pos(0),
    highest_upstream_sync_pos(0),
    totalBufferCapacityInFrames(0),
    frameSize(0),
    totalWrittenFrames(0) {}

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
    size_t buffer_pos = 0;
    for (size_t i = 0; i < numFrames; i++) {
        if (upstream_pos < head_upstream_sync_pos) {
            std::cout << "out of range: upstream_pos < head_upstream_sync_pos" << std::endl;
            std::cout << "upstream_pos: " << upstream_pos << std::endl;
            upstream_pos++;
            continue;
        }

        buffer_pos = (upstream_pos - head_upstream_sync_pos) % totalBufferCapacityInFrames;
        std::cout << "buffer_pos: " << buffer_pos << std::endl;

        if (tail_upstream_sync_pos < upstream_pos) { 
            std::cout << "out of range: tail_upstream_sync_pos < upstream_pos && IsBufferFull()" << std::endl;
            std::cout << "upstream_pos: " << upstream_pos << std::endl;
            break;
        }
            
        std::memcpy(&buffer[buffer_pos * frameSize], data + i * frameSize, frameSize);
        if (highest_upstream_sync_pos < upstream_pos) {
            highest_upstream_sync_pos = upstream_pos;
        }
        totalWrittenFrames++;
        upstream_pos++;
        
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
        std::fill(buffer.begin() + buffer_pos * frameSize, buffer.begin() + (buffer_pos + 1) * frameSize, 0);
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
    if (0 == totalBufferCapacityInFrames) {
        size_t nBlockAlign = packet->Channels * packet->BitsPerSample / 8;
        size_t frames = CalculateFramesForDurationSeconds(packet->SamplingRate, packet->Channels, 5);

        buffer.resize(frames * nBlockAlign);
        totalBufferCapacityInFrames = frames;
        frameSize = nBlockAlign;
    }
    Write(packet->UpstreamDevicePosition, packet->Data.get(), packet->Frames);
}
