#include <iostream>
#include <atlbase.h>
#include <atlcom.h>
#include "RingBuffer.h"
#include "AudioCapture.h"
#include "WavFileWriter.h"
#include "Utility.h"


int main()
{
    std::cout << "SoundStreamLink" << std::endl;
    std::cout << std::endl;
    std::cout << "Main Thread Initializing" << std::endl;
    
    HRESULT hr = CoInitialize(NULL);
    CheckHresult(hr, "CoInitialize");

    CComPtr<IMMDeviceEnumerator> pEnumerator = NULL;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);
    CheckHresult(hr, "CoCreateInstance");

    CComPtr<IMMDevice> pDevice = NULL;
    hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
    CheckHresult(hr, "pEnumerator->GetDefaultAudioEndpoint");

    CComPtr<IAudioClient> pAudioClient = NULL;
    hr = pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&pAudioClient);
    CheckHresult(hr, "pDevice->Activate");

    std::cout << "Getting AudioClient's MixFormat:" << std::endl;
    CComHeapPtr<WAVEFORMATEX> pFormat;
    hr = pAudioClient->GetMixFormat(&pFormat);
    CheckHresult(hr, "pAudioClient->GetMixFormat");

    std::cout << "Format:" << std::endl;
    std::cout << "WaveFormat.wFormatTag: " << pFormat->wFormatTag << std::endl;
    std::cout << "WaveFormat.nChannels: " << pFormat->nChannels << std::endl;
    std::cout << "WaveFormat.nSamplesPerSec: " << pFormat->nSamplesPerSec << std::endl;
    std::cout << "WaveFormat.wBitsPerSample: " << pFormat->wBitsPerSample << std::endl;
    std::cout << "WaveFormat.nAvgBytesPerSec: " << pFormat->nAvgBytesPerSec << std::endl;
    std::cout << "WaveFormat.nBlockAlign: " << pFormat->nBlockAlign << std::endl;
    std::cout << "---" << std::endl;

    const REFERENCE_TIME hnsBufferDuration = 100 * 10000; // 100 ms = 100 * 10^4 in 100ns units
    std::cout << "hnsRequestedDuration (in 100ns): " << hnsBufferDuration << " (" << hnsBufferDuration / 10000 << "ms)" << std::endl;

    hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_LOOPBACK, hnsBufferDuration, 0, pFormat, NULL);
    CheckHresult(hr, "pAudioClient->Initialize");

    REFERENCE_TIME hnsStreamLatency;
    hr = pAudioClient->GetStreamLatency(&hnsStreamLatency);
    std::cout << "GetStreamLatency: " << hnsStreamLatency << " (" << hnsStreamLatency / 10000 << "ms)" << std::endl;

    REFERENCE_TIME hnsDefaultDevicePeriod;
    REFERENCE_TIME hnsMinimumDevicePeriod;
    hr = pAudioClient->GetDevicePeriod(&hnsDefaultDevicePeriod, &hnsMinimumDevicePeriod);
    std::cout << "hnsDefaultDevicePeriod (in 100ns): " << hnsDefaultDevicePeriod << " (" << hnsDefaultDevicePeriod / 10000 << "ms)" << std::endl;
    std::cout << "hnsMinimumDevicePeriod (in 100ns): " << hnsMinimumDevicePeriod << " (" << hnsMinimumDevicePeriod / 10000 << "ms)" << std::endl;

    auto audioCapture = std::make_unique<AudioCapture>(pAudioClient);

    std::cout << "Capture Starting\n";
    audioCapture->Start();
    std::cout << "Capture Started\n";

    // 例として10秒間キャプチャ
    const size_t CAPTURE_DURATION_SECONDS = 10;
    Sleep(CAPTURE_DURATION_SECONDS * 1000);

    std::cout << "Capture Finising" << std::endl;
    audioCapture->Stop();
    std::cout << "Capture Finished" << std::endl;

    std::cout << "wav file saving" << std::endl;
    size_t neededCapacity = audioCapture->GetCurrentValidFrames() * pFormat->nBlockAlign;
    std::cout << "neededCapacity: " << neededCapacity << std::endl;
    std::vector<BYTE> readBuffer(neededCapacity, 0);
    std::cout << "readBuffer.size(): " << readBuffer.size() << std::endl;
    size_t readFrames = audioCapture->BufferReadAll(readBuffer.data());
    WavFileWriter::WriteWaveFile("output.wav", readBuffer, *pFormat);
    std::cout << "wav file saved" << std::endl;

    CoUninitialize();
    return 0;
}
