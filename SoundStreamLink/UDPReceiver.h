#pragma once

#include <iostream>
#include <thread>
#include <vector>
#include <winsock2.h>
# include <ws2tcpip.h>
#include "AudioPacket.h"
#include "IUDPReceiverBufferPreparedEventListener.h"

class UDPReceiver {
private:
    SOCKET udpSocket;
    sockaddr_in listenAddr;
    static const int BUFFER_SIZE = 65536;
    std::shared_ptr<char[]> buffer;

    bool listening;
    std::thread listenThread;

    // listener feature
    std::vector<std::shared_ptr<IUDPReceiverBufferPreparedEventListener>> listeners;

    void ListenLoop();
    bool DecodeAudioPacket(const char* data, int dataSize, AudioPacket& packet);
public:
    UDPReceiver(unsigned short listenPort);
    ~UDPReceiver();

    void StartListening();
    void StopListening();

    void AddListener(std::shared_ptr<IUDPReceiverBufferPreparedEventListener>&& newListener);
    void ClearListeners();
};
