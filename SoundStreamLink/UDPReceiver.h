#pragma once

#include <winsock2.h>
#include "AudioPacket.h"

class UDPReceiver {
private:
    SOCKET udpSocket;
    sockaddr_in localAddr;

public:
    UDPReceiver(unsigned short localPort);
    ~UDPReceiver();
    bool receive(AudioPacket& packet);
};
