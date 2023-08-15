#include <iostream>
#include <atlbase.h>
#include <atlcom.h>
#include "RingBuffer.h"
#include "AudioCapture.h"
#include "WavFileWriter.h"
#include "Utility.h"

void Debug(std::string debugFile) {
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
    WavFileWriter::WriteWaveFile(debugFile, readBuffer, *pFormat);
    std::cout << "wav file saved" << std::endl;

    CoUninitialize();
}

void PrintUsage() {
    std::cout << "SoundStreamLink.exe  [mode option] [other options]" << std::endl;
    std::cout << "modes (mandatory):" << std::endl;
    std::cout << "-s     Server" << std::endl;
    std::cout << "-c     Client" << std::endl;
    std::cout << "-d     Debug" << std::endl;
    std::cout << "options (complementary):" << std::endl;
    std::cout << "-sa    For client mode. Specify server address." << std::endl;
    std::cout << "-sp    For server and client mode. Specify server port." << std::endl;
    std::cout << "-ca    For server mode. Specify client address." << std::endl;
    std::cout << "-cp    For server and client mode. Specify client port." << std::endl;
    std::cout << "-df    For debug mode. Specify output path." << std::endl;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Error! No arguments. To start the program, please give a mode option (-s or -c)." << std::endl << std::endl;
        PrintUsage();
        return 1;
    }

    bool debugMode = false;
    bool serverMode = false;
    bool clientMode = false;
    std::string debugFile;
    std::string serverAddress;
    int serverPort = 0;
    std::string clientAddress;
    int clientPort = 0;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "-d") {
            debugMode = true;
        }
        else if (arg == "-s") {
            serverMode = true;
        }
        else if (arg == "-c") {
            clientMode = true;
        }
        else if (arg == "-df" && i + 1 < argc) {
            debugFile = argv[++i];
        }
        else if (arg == "-sa" && i + 1 < argc) {
            serverAddress = argv[++i];
        }
        else if (arg == "-sp" && i + 1 < argc) {
            serverPort = std::stoi(argv[++i]);
        }
        else if (arg == "-ca" && i + 1 < argc) {
            clientAddress = argv[++i];
        }
        else if (arg == "-cp" && i + 1 < argc) {
            clientPort = std::stoi(argv[++i]);
        }
    }

    if (debugMode + serverMode + clientMode != 1) {
        std::cerr << "Error! Conflicted Modes. To start the program, please give the only one mode option (-s or -c)." << std::endl << std::endl;
        PrintUsage();
        return 1;
    }

    if (debugMode) {
        std::cout << "Mode: Debug" << std::endl;
        if (!debugFile.empty()) {
            debugFile = "debug.wav";
        }
        std::cout << "Debug File: " << debugFile << std::endl;
        Debug(debugFile);
    }
    else if (serverMode) {
        std::cout << "Mode: Server" << std::endl;
        if (!serverAddress.empty()) {
            std::cout << "サーバーアドレス: " << serverAddress << std::endl;
        }
        if (serverPort) {
            std::cout << "サーバーポート: " << serverPort << std::endl;
        }
    }
    else if (clientMode) {
        std::cout << "Mode: Client" << std::endl;
        if (!clientAddress.empty()) {
            std::cout << "クライアントアドレス: " << clientAddress << std::endl;
        }
        if (clientPort) {
            std::cout << "クライアントポート: " << clientPort << std::endl;
        }
    }

    return 0;
}
