#include "AudioCapture.h"
#include <avrt.h>
#pragma comment(lib, "avrt.lib")
#include "Utility.h"


DWORD WINAPI AudioCapture::WASAPICaptureThreadWrapper(LPVOID pThis) {
    AudioCapture* audioCapture = static_cast<AudioCapture*>(pThis);
    return audioCapture->WASAPICaptureThread();
}

DWORD AudioCapture::WASAPICaptureThread() {
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
        std::cout << "flags: " << std::hex << flags << std::oct << std::endl;
        std::cout << "flags->AUDCLNT_BUFFERFLAGS_SILENT: " << (flags & AUDCLNT_BUFFERFLAGS_SILENT) << std::endl;
        std::cout << "flags->AUDCLNT_BUFFERFLAGS_DATA_DISCONTINUITY: " << (flags & AUDCLNT_BUFFERFLAGS_DATA_DISCONTINUITY) << std::endl;
        std::cout << "flags->AUDCLNT_BUFFERFLAGS_TIMESTAMP_ERROR: " << (flags & AUDCLNT_BUFFERFLAGS_TIMESTAMP_ERROR) << std::endl;
        std::cout << "u64DevicePosition: " << u64DevicePosition << std::endl;
        std::cout << "u64QPCPosition: " << u64QPCPosition << std::endl;

        std::cout << "Buffer Statistics:" << std::endl;
        std::cout << "Buffer->TotalWrittenFrames: " << ringBuffer->GetTotalWrittenFrames() << std::endl;
        std::cout << "Buffer->TotalZeroFilledFrames: " << ringBuffer->GetTotalZeroFilledFrames() << std::endl;
        std::cout << "Buffer->CurrentValidFrames: " << ringBuffer->GetCurrentValidFrames() << std::endl;
        std::cout << "Buffer->CurrentZeroFilledFrames: " << ringBuffer->GetCurrentZeroFilledFrames() << std::endl;
#endif // DEBUG
        if (numFramesToRead > 0) {
            mtx.lock();
            ringBuffer->Write(u64DevicePosition, pData, numFramesToRead);
            mtx.unlock();

            hr = pCaptureClient->ReleaseBuffer(numFramesToRead);
            CheckHresult(hr, "pCaptureClient->ReleaseBuffer");
        }
        Sleep(1);
    }
    return 0;
}

void AudioCapture::startCaptureThread() {
    HANDLE handle = CreateThread(NULL, 0, WASAPICaptureThreadWrapper, this, 0, NULL);
    CheckHandle(handle, "CreateThead for WASAPICaptureThreadWrapper");
}

AudioCapture::AudioCapture(CComPtr<IAudioClient> audioClient)
    : pAudioClient(audioClient) {
    initialized = false;
    terminated = false;

    HRESULT hr = pAudioClient->GetService(__uuidof(IAudioCaptureClient), (void**)&pCaptureClient);
    CheckHresult(hr, "pAudioClient->GetService");

    hr = pAudioClient->GetMixFormat(&pFormat);
    CheckHresult(hr, "pAudioClient->GetMixFormat");

    const size_t MAX_CAPTURE_FRAMES = CalculateFramesForDurationSeconds(*pFormat, 5);
    ringBuffer = std::make_unique<RingBuffer>(*pFormat, MAX_CAPTURE_FRAMES);
}

size_t AudioCapture::CalculateFramesForDurationSeconds(const WAVEFORMATEX& format, double seconds) {
    size_t totalSamples = static_cast<size_t>(seconds * format.nSamplesPerSec);
    size_t samplesPerFrame = format.nBlockAlign / (format.wBitsPerSample / 8 * format.nChannels);
    size_t totalFrames = totalSamples / samplesPerFrame;
    return totalFrames;
}

void AudioCapture::Start() {
    assert(initialized == false);
    assert(terminated == false);
    initialized = true;
    HRESULT hr = pAudioClient->Start();
    CheckHresult(hr, "pAudioClient->Start");

    std::cout << "CaptureThread is starting" << std::endl;
    startCaptureThread();
    std::cout << "CaptureThread has been started" << std::endl;
}

void AudioCapture::Stop() {
    assert(initialized == true);
    assert(terminated == false);
    HRESULT hr = pAudioClient->Stop();
    CheckHresult(hr, "pAudioClient->Stop");

    terminated = true;
}

size_t AudioCapture::BufferRead(UINT64 u64DevicePosition, BYTE* output, size_t numFrames) {
    mtx.lock();
    size_t ret = ringBuffer->Read(u64DevicePosition, output, numFrames);
    mtx.unlock();
    return ret;
}

size_t AudioCapture::BufferReadAll(BYTE* output) {
    mtx.lock();
    size_t ret = ringBuffer->ReadAll(output);
    mtx.unlock();
    return ret;
}

size_t AudioCapture::GetTotalWrittenFrames() const {
    return ringBuffer->GetTotalWrittenFrames();
}

size_t AudioCapture::GetTotalZeroFilledFrames() const {
    return ringBuffer->GetTotalZeroFilledFrames();
}

size_t AudioCapture::GetCurrentValidFrames() const {
    return ringBuffer->GetCurrentValidFrames();
}

size_t AudioCapture::GetCurrentZeroFilledFrames() const {
    return ringBuffer->GetCurrentZeroFilledFrames();
}


void AudioCapture::addUpdateListener(std::unique_ptr<IBufferUpdateListener>&& newListener) {
    ringBuffer->addUpdateListener(std::move(newListener));
}

void AudioCapture::clearUpdateListeners() {
    ringBuffer->clearUpdateListeners();
}