#pragma once

#include <atlbase.h>
#include <atlcom.h>
#include <Audioclient.h>
#include <mutex>
#include "RingBuffer.h"

class AudioCapture {
private:
    CComPtr<IAudioClient> pAudioClient;
    CComPtr<IAudioCaptureClient> pCaptureClient;
    WAVEFORMATEX* pFormat;
    std::unique_ptr<RingBuffer> ringBuffer;
    std::mutex mtx;
    bool initialized;
    bool terminated;

    static DWORD WINAPI WASAPICaptureThreadWrapper(LPVOID pThis);
    DWORD WASAPICaptureThread();
    void startCaptureThread();

public:
    AudioCapture(CComPtr<IAudioClient> audioClient);
    size_t CalculateFramesForDurationSeconds(const WAVEFORMATEX& format, double seconds);
    void Start();
    void Stop();
    size_t BufferRead(UINT64 u64DevicePosition, BYTE* output, UINT32 numFrames);
    size_t BufferReadAll(BYTE* output);
    size_t GetTotalWrittenFrames() const;
    size_t GetTotalZeroFilledFrames() const;
    size_t GetCurrentValidFrames() const;
    size_t GetCurrentZeroFilledFrames() const;

    void addUpdateListener(std::unique_ptr<IBufferUpdateListener>&& newListener);
    void clearUpdateListeners();
};
