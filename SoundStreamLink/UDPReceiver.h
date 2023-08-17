#pragma once

#include <iostream>
#include <thread>
#include <winsock2.h>
# include <ws2tcpip.h>
#include "AudioPacket.h"

class UDPReceiver {
private:
    SOCKET udpSocket;
    sockaddr_in listenAddr;
    static const int BUFFER_SIZE = 65536;
    std::unique_ptr<char[]> buffer;

    bool listening;
    std::thread listenThread;

    void listenLoop();
    bool DecodeAudioPacket(const char* data, int dataSize, AudioPacket& packet);
public:
    UDPReceiver(unsigned short listenPort);
    ~UDPReceiver();

    void startListening();
    void stopListening();
};
