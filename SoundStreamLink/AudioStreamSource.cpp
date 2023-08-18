#include "AudioStreamSource.h"
#include <iostream>
#include <cassert>
#include <bitset>
#include <avrt.h>
#pragma comment(lib, "avrt.lib")
#include "Utility.h"

DWORD WINAPI AudioStreamSource::WASAPICaptureThreadWrapper(LPVOID pThis) {
    AudioStreamSource* audioCapture = static_cast<AudioStreamSource*>(pThis);
    return audioCapture->WASAPICaptureThread();
}

DWORD AudioStreamSource::WASAPICaptureThread() {
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
    return 0;
}

void AudioStreamSource::startCaptureThread() {
    HANDLE handle = CreateThread(NULL, 0, WASAPICaptureThreadWrapper, this, 0, NULL);
    CheckHandle(handle, "CreateThead for WASAPICaptureThreadWrapper");
}

AudioStreamSource::AudioStreamSource(CComPtr<IAudioClient> audioClient)
    : pAudioClient(audioClient) {
    initialized = false;
    terminated = false;

    HRESULT hr = pAudioClient->GetService(__uuidof(IAudioCaptureClient), (void**)&pCaptureClient);
    CheckHresult(hr, "pAudioClient->GetService");

    hr = pAudioClient->GetMixFormat(&pFormat);
    CheckHresult(hr, "pAudioClient->GetMixFormat");

    const size_t MAX_CAPTURE_FRAMES = CalculateFramesForDurationSeconds(pFormat->nSamplesPerSec, pFormat->nChannels, 5);
}

void AudioStreamSource::Start() {
    assert(initialized == false);
    assert(terminated == false);
    initialized = true;
    HRESULT hr = pAudioClient->Start();
    CheckHresult(hr, "pAudioClient->Start");

    std::cout << "CaptureThread is starting" << std::endl;
    startCaptureThread();
    std::cout << "CaptureThread has been started" << std::endl;
}

void AudioStreamSource::Stop() {
    assert(initialized == true);
    assert(terminated == false);
    HRESULT hr = pAudioClient->Stop();
    CheckHresult(hr, "pAudioClient->Stop");

    terminated = true;
}

void AudioStreamSource::AddListener(std::shared_ptr<IAudioStreamSourceBufferPreparedEventListener>&& newListener) {
    listeners.push_back(newListener);
}

void AudioStreamSource::ClearListeners() {
    listeners.clear();
}
