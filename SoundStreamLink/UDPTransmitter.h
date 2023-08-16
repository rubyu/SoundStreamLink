#pragma once

#include <string>
#include <memory>
#include <winsock2.h>
#include <Audioclient.h>
#include "AudioPacket.h"
#include "IBufferUpdateListener.h"

class UDPTransmitter : public IBufferUpdateListener {
private:
    SOCKET udpSocket;
    sockaddr_in destAddr;

    std::unique_ptr<AudioPacket> packet;

public:
    UDPTransmitter(WAVEFORMATEX* _waveFormat, std::string destIP, unsigned short destPort);
    ~UDPTransmitter();

    void bufferUpdated(UINT64 u64DevicePosition, BYTE* data, size_t numFrames);
};
