#pragma once

#include <memory>
#include <vector>
#include <atlbase.h>
#include <atlcom.h>
#include <Audioclient.h>
#include "IAudioStreamSourceBufferPreparedEventListener.h"
#include "AudioStreamSink.h"

class AudioRenderer {
private:
    bool initialized;
    bool terminated;

    std::shared_ptr<AudioStreamSink> sink;

    static DWORD WINAPI WASAPIRenderThreadWrapper(LPVOID pThis);
    DWORD WASAPIRenderThread();
    void StartRenderThread();

public:
    AudioRenderer(std::shared_ptr<AudioStreamSink> streamSink);
    void Start();
    void Stop();
};
