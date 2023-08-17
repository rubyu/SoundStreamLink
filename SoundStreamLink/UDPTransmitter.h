#pragma once

#include <string>
#include <vector>
#include <memory>
#include <winsock2.h>
#include <Audioclient.h>
#include "AudioPacket.h"
#include "IBufferUpdateListener.h"

class UDPTransmitter : public IBufferUpdateListener {
private:
    SOCKET udpSocket;
    sockaddr_in destAddr;

    AudioPacket packet{};

    std::vector<BYTE> buffer;

public:
    UDPTransmitter(WAVEFORMATEX* _waveFormat, std::string destIP, unsigned short destPort);
    ~UDPTransmitter();

    void bufferUpdateCallback(UINT64 u64DevicePosition, BYTE* data, UINT32 numFrames);
};
