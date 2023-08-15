#pragma once

#include <string>
#include <winsock2.h>
#include <Audioclient.h>
#include "AudioPacket.h"
#include "IBufferUpdateListener.h"

class UDPTransmitter : public IBufferUpdateListener {
private:
    SOCKET udpSocket;
    sockaddr_in destAddr;

    AudioPacket packet;

public:
    UDPTransmitter(WAVEFORMATEX* _waveFormat, std::string destIP, unsigned short destPort);
    ~UDPTransmitter();

    void bufferUpdated(UINT64 u64DevicePosition, BYTE* data, size_t numFrames);
};