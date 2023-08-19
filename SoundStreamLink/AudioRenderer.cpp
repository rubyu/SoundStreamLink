#include "AudioRenderer.h"
#include <iostream>
#include <cassert>
#include <bitset>
#include <avrt.h>
#pragma comment(lib, "avrt.lib")
#include "Utility.h"

DWORD WINAPI AudioRenderer::WASAPIRenderThreadWrapper(LPVOID pThis) {
    AudioRenderer* renderer = static_cast<AudioRenderer*>(pThis);
    return renderer->WASAPIRenderThread();
}

DWORD AudioRenderer::WASAPIRenderThread() {
    HRESULT hr = CoInitialize(NULL);
    CheckHresult(hr, "CoInitialize");

    while (true) {
        if (0 < sink->GetTotalWrittenFrames()) break;
        std::cout << "Waiting the first packet..." << std::endl;
        Sleep(1000);
    }

    const size_t nBlockAlign = sink->GetChannels() * sink->GetBitsPerSample() / 8;
    const size_t bufferMs = 200; //ms
    const size_t bufferFrames = CalculateFramesForDurationMilliseconds(sink->GetSamplingRate(), sink->GetChannels(), bufferMs); //frames

    while (true) {
        if (bufferFrames < sink->GetSyncedFrames()) break;
        std::cout << "Waiting the buffer filled..." << std::endl;
        Sleep(1000);
    }

    IMMDeviceEnumerator* pEnumerator = NULL;
    IMMDevice* pDevice = NULL;
    IAudioClient* pAudioClient = NULL;
    IAudioRenderClient* pRenderClient = NULL;

    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);
    CheckHresult(hr, "CoCreateInstance");

    hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
    CheckHresult(hr, "pEnumerator->GetDefaultAudioEndpoint");

    hr = pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&pAudioClient);
    CheckHresult(hr, "pDevice->Activate");

    WAVEFORMATEX* pwfx = NULL;
    hr = pAudioClient->GetMixFormat(&pwfx);
    CheckHresult(hr, "pAudioClient->GetMixFormat");

    std::cout << "Current:" << std::endl;
    std::cout << "WaveFormat.wFormatTag: " << pwfx->wFormatTag << std::endl;
    std::cout << "WaveFormat.nChannels: " << pwfx->nChannels << std::endl;
    std::cout << "WaveFormat.nSamplesPerSec: " << pwfx->nSamplesPerSec << std::endl;
    std::cout << "WaveFormat.wBitsPerSample: " << pwfx->wBitsPerSample << std::endl;
    std::cout << "WaveFormat.nAvgBytesPerSec: " << pwfx->nAvgBytesPerSec << std::endl;
    std::cout << "WaveFormat.nBlockAlign: " << pwfx->nBlockAlign << std::endl;
    std::cout << "---" << std::endl;

    pwfx->nSamplesPerSec = sink->GetSamplingRate();
    pwfx->nChannels = sink->GetChannels();
    pwfx->wBitsPerSample = sink->GetBitsPerSample();

    std::cout << "Want:" << std::endl;
    std::cout << "WaveFormat.wFormatTag: " << pwfx->wFormatTag << std::endl;
    std::cout << "WaveFormat.nChannels: " << pwfx->nChannels << std::endl;
    std::cout << "WaveFormat.nSamplesPerSec: " << pwfx->nSamplesPerSec << std::endl;
    std::cout << "WaveFormat.wBitsPerSample: " << pwfx->wBitsPerSample << std::endl;
    std::cout << "WaveFormat.nAvgBytesPerSec: " << pwfx->nAvgBytesPerSec << std::endl;
    std::cout << "WaveFormat.nBlockAlign: " << pwfx->nBlockAlign << std::endl;
    std::cout << "---" << std::endl;

    const REFERENCE_TIME hnsBufferDuration = 100 * 10000; // 100 ms = 100 * 10^4 in 100ns units
    std::cout << "hnsRequestedDuration (in 100ns): " << hnsBufferDuration << " (" << hnsBufferDuration / 10000 << "ms)" << std::endl;

    hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, hnsBufferDuration, 0, pwfx, NULL);
    CheckHresult(hr, "pAudioClient->Initialize");

    hr = pAudioClient->GetService(__uuidof(IAudioRenderClient), (void**)&pRenderClient);
    CheckHresult(hr, "pAudioClient->GetService");

    const size_t upstream_buffer_size = bufferFrames * nBlockAlign;
    std::vector<BYTE> upstream_buffer(bufferFrames * nBlockAlign, 0);
    BYTE* downstream_buffer = NULL;

    // truncate the upstream buffer size to bufferFrames.
    sink->CommitRead(sink->GetSyncedFrames() - bufferFrames);

    hr = pAudioClient->Start();
    CheckHresult(hr, "pAudioClient->Start()");


    std::cout << "Start playing" << std::endl;
    while (!terminated) {
        UINT32 padding = 0;
        hr = pAudioClient->GetCurrentPadding(&padding);
        CheckHresult(hr, "pAudioClient->GetCurrentPadding()");

        UINT32 availableSpace = upstream_buffer_size / pwfx->nBlockAlign - padding; // frames
        if (0 < availableSpace) {
            std::cout << "availableSpace: " << availableSpace << std::endl;
            sink->Read(upstream_buffer.data(), availableSpace);
            std::cout << "sink->Read(): " << availableSpace << std::endl;

            hr = pRenderClient->GetBuffer(availableSpace, &downstream_buffer);
            CheckHresult(hr, "pRenderClient->GetBuffer()");

            memcpy(downstream_buffer, upstream_buffer.data(), availableSpace * pwfx->nBlockAlign);
            hr = pRenderClient->ReleaseBuffer(availableSpace, 0);
            CheckHresult(hr, "pRenderClient->ReleaseBuffer()");

            std::cout << "pRenderClient->ReleaseBuffer(): " << availableSpace << std::endl;
        }
    }

    hr = pAudioClient->Stop();
    CheckHresult(hr, "pAudioClient->Stop()");

    CoUninitialize();

    /*

    HRESULT hr = CoInitialize(NULL);
    CheckHresult(hr, "CoInitialize");

    DWORD taskIndex = 0;
    HANDLE handle = AvSetMmThreadCharacteristics(L"Audio", &taskIndex);
    CheckHandle(handle, "AvSetMmThreadCharacteristics");
    std::cout << "taskIndex: " << taskIndex << std::endl;



    UINT32 numFramesToRead = 0;
    UINT64 u64DevicePosition = 0;
    UINT64 u64QPCPosition = 0;
    DWORD flags = NULL;
    BYTE* pData = NULL;
    while (!terminated)
    {
        hr = pCaptureClient->GetBuffer(&pData, &numFramesToRead, &flags, &u64DevicePosition, &u64QPCPosition);
        CheckHresult(hr, "pCaptureClient->GetBuffer");
#ifdef DEBUG
        std::cout << "numFramesToRead: " << numFramesToRead << std::endl;
        std::cout << "numFramesToRead (in milliseconds): " << static_cast<double>(numFramesToRead) / pFormat->nSamplesPerSec * 1000 << std::endl;
        std::cout << "flags: " << std::bitset<32>(flags) << std::dec << std::endl;
        std::cout << "flags->AUDCLNT_BUFFERFLAGS_SILENT: " << (flags & AUDCLNT_BUFFERFLAGS_SILENT) << std::endl;
        std::cout << "flags->AUDCLNT_BUFFERFLAGS_DATA_DISCONTINUITY: " << (flags & AUDCLNT_BUFFERFLAGS_DATA_DISCONTINUITY) << std::endl;
        std::cout << "flags->AUDCLNT_BUFFERFLAGS_TIMESTAMP_ERROR: " << (flags & AUDCLNT_BUFFERFLAGS_TIMESTAMP_ERROR) << std::endl;
        std::cout << "u64DevicePosition: " << u64DevicePosition << std::endl;
        std::cout << "u64QPCPosition: " << u64QPCPosition << std::endl;
#endif // DEBUG
        if (numFramesToRead > 0) {
            for (auto& listener : listeners) {
                listener->AudioStreamSourceBufferPreparedCallback(u64DevicePosition, pData, numFramesToRead);
            }

            hr = pCaptureClient->ReleaseBuffer(numFramesToRead);
            CheckHresult(hr, "pCaptureClient->ReleaseBuffer");
        }
        Sleep(1);
    }
    */

    return 0;
}

void AudioRenderer::StartRenderThread() {
    HANDLE handle = CreateThread(NULL, 0, WASAPIRenderThreadWrapper, this, 0, NULL);
    CheckHandle(handle, "CreateThead for WASAPICaptureThreadWrapper");
}

AudioRenderer::AudioRenderer(std::shared_ptr<AudioStreamSink> streamSink)
    : sink(streamSink),
    initialized(false), 
    terminated(false) {}

void AudioRenderer::Start() {
    assert(initialized == false);
    assert(terminated == false);
    initialized = true;

    std::cout << "RenderThread is starting" << std::endl;
    StartRenderThread();
    std::cout << "RenderThread has been started" << std::endl;
}

void AudioRenderer::Stop() {
    assert(initialized == true);
    assert(terminated == false);

    terminated = true;
}
