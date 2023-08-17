#include "RingBuffer.h"

void RingBuffer::IncrementHead() {
    if (zeroFilledPositions.count(head)) {
        zeroFilledPositions.erase(head);
        currentZeroFilledFrames--;
    }
    head = (head + 1) % totalBufferCapacityInFrames;
    head_upstream_sync_pos++;
}

void RingBuffer::IncrementTail() {
    tail = (tail + 1) % totalBufferCapacityInFrames;
    tail_upstream_sync_pos++;
    totalWrittenFrames++;
    if (head == tail) {
        IncrementHead();
    }
}

void RingBuffer::IncrementTailZerofill() {
    totalZeroFilledFrames++;
    currentZeroFilledFrames++;
    zeroFilledPositions.insert(tail);
    IncrementTail();
}

bool RingBuffer::IsInitialState() {
    return head == tail;
}

RingBuffer::RingBuffer(const WAVEFORMATEX& format, size_t numFrames)
    : buffer(numFrames* format.nBlockAlign),
    head(0),
    head_upstream_sync_pos(0),
    tail(0),
    tail_upstream_sync_pos(0),
    totalBufferCapacityInFrames(numFrames),
    frameSize(format.nBlockAlign),
    totalWrittenFrames(0),
    totalZeroFilledFrames(0),
    currentZeroFilledFrames(0) {}

void RingBuffer::Write(UINT64 u64DevicePosition, BYTE* data, UINT32 numFrames) {
    if (numFrames == 0) return;

    for (auto& listener : updateListeners) {
        listener->bufferUpdateCallback(u64DevicePosition, data, numFrames);
    }

    if (IsInitialState()) {
        head_upstream_sync_pos = u64DevicePosition;
        tail_upstream_sync_pos = u64DevicePosition;
        for (size_t i = 0; i < numFrames; i++) {
            std::memcpy(&buffer[tail * frameSize], data + i * frameSize, frameSize);
            IncrementTail();
        }
        assert(tail_upstream_sync_pos == u64DevicePosition + numFrames);
    }
    else {
        assert(head != tail);
        size_t upstream_diff = u64DevicePosition - tail_upstream_sync_pos;
        if (upstream_diff > 0) {
            std::cout << "Lost frames found: " << upstream_diff << std::endl;
            for (size_t i = 0; i < upstream_diff; i++) {
                std::memset(&buffer[tail * frameSize], 0, frameSize);
                IncrementTailZerofill();
            }
            assert(tail_upstream_sync_pos == u64DevicePosition);
        }
        for (size_t i = 0; i < numFrames; i++) {
            std::memcpy(&buffer[tail * frameSize], data + i * frameSize, frameSize);
            IncrementTail();
        }
        assert(tail_upstream_sync_pos == u64DevicePosition + numFrames);
    }

    std::cout << "head: " << head << std::endl;
    std::cout << "head_upstream_sync_pos: " << head_upstream_sync_pos << std::endl;
    std::cout << "tail: " << tail << std::endl;
    std::cout << "tail_upstream_sync_pos: " << tail_upstream_sync_pos << std::endl;
}

size_t RingBuffer::Read(UINT64 u64DevicePosition, BYTE* output, UINT32 numFrames) {
    assert(head != tail);
    assert(head_upstream_sync_pos <= u64DevicePosition);
    assert(u64DevicePosition <= tail_upstream_sync_pos);
    assert(numFrames <= totalBufferCapacityInFrames);
    size_t pos = (head + u64DevicePosition - head_upstream_sync_pos) % totalBufferCapacityInFrames;
    for (size_t i = 0; i < numFrames; i++) {
        std::memcpy(output + i * frameSize, &buffer[pos * frameSize], frameSize);
        pos = (pos + 1) % totalBufferCapacityInFrames;
    }
    return numFrames;
}

size_t RingBuffer::ReadAll(BYTE* output) {
    assert(head != tail);
    size_t pos = head;
    size_t framesToRead = GetCurrentValidFrames();
    for (size_t i = 0; i < framesToRead; i++) {
        std::memcpy(output + i * frameSize, &buffer[pos * frameSize], frameSize);
        pos = (pos + 1) % totalBufferCapacityInFrames;
    }
    return framesToRead;
}

size_t RingBuffer::GetTotalWrittenFrames() const {
    return totalWrittenFrames;
}

size_t RingBuffer::GetTotalZeroFilledFrames() const {
    return totalZeroFilledFrames;
}

size_t RingBuffer::GetCurrentValidFrames() const {
    if (head == tail) return 0;
    std::cout << "GetCurrentValidFrames()" << std::endl;
    std::cout << "head: " << head << std::endl;
    std::cout << "tail: " << tail << std::endl;
    std::cout << "totalBufferCapacityInFrames: " << totalBufferCapacityInFrames << std::endl;
    std::cout << "result: " << (head < tail ? tail - head : totalBufferCapacityInFrames - head + tail) << std::endl;
    return head < tail ? tail - head : totalBufferCapacityInFrames - head + tail;
}

size_t RingBuffer::GetCurrentZeroFilledFrames() const {
    return currentZeroFilledFrames;
}

void RingBuffer::addUpdateListener(std::unique_ptr<IBufferUpdateListener>&& newListener) {
    updateListeners.push_back(std::move(newListener));
}

void RingBuffer::clearUpdateListeners() {
    updateListeners.clear();
}
