#include "UDPReceiver.h"
#pragma comment(lib, "ws2_32.lib")
#include "Utility.h"

UDPReceiver::UDPReceiver(unsigned short localPort) {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    MustEquals(0, result, "WSAStartup");

    udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    CheckSocket(udpSocket, "socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)");

    memset(&localAddr, 0, sizeof(localAddr));
    localAddr.sin_family = AF_INET;
    localAddr.sin_port = htons(localPort);
    localAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(udpSocket, (struct sockaddr*)&localAddr, sizeof(localAddr));
}

UDPReceiver::~UDPReceiver() {
    closesocket(udpSocket);
    WSACleanup();
}

bool UDPReceiver::receive(AudioPacket& packet) {
    /*
    BYTE buffer[8192]; // 仮の最大サイズ。必要に応じて調整してください。
    int bytesReceived = recvfrom(udpSocket, (char*)buffer, sizeof(buffer), 0, nullptr, nullptr);
    if (bytesReceived == SOCKET_ERROR) {
        return false;
    }

    BYTE* ptr = buffer;
    memcpy(&packet.SamplingRate, ptr, sizeof(packet.SamplingRate)); ptr += sizeof(packet.SamplingRate);
    memcpy(&packet.Channels, ptr, sizeof(packet.Channels)); ptr += sizeof(packet.Channels);
    // ... [同様の操作を他のメンバ変数に対して続けます]
    packet.Data = new BYTE[packet.DataSize];
    memcpy(packet.Data, ptr, packet.DataSize);
    */
    return true;
}
