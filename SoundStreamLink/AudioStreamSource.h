#pragma once

#include <memory>
#include <vector>
#include <atlbase.h>
#include <atlcom.h>
#include <Audioclient.h>
#include "IAudioStreamSourceBufferPreparedEventListener.h"

class AudioStreamSource {
private:
    CComPtr<IAudioClient> pAudioClient;
    CComPtr<IAudioCaptureClient> pCaptureClient;
    WAVEFORMATEX* pFormat;
    bool initialized;
    bool terminated;

    // listener feature
    std::vector<std::shared_ptr<IAudioStreamSourceBufferPreparedEventListener>> listeners;

    static DWORD WINAPI WASAPICaptureThreadWrapper(LPVOID pThis);
    DWORD WASAPICaptureThread();
    void startCaptureThread();

public:
    AudioStreamSource(CComPtr<IAudioClient> audioClient);
    void Start();
    void Stop();

    void AddListener(std::shared_ptr<IAudioStreamSourceBufferPreparedEventListener>&& newListener);
    void ClearListeners();
};
