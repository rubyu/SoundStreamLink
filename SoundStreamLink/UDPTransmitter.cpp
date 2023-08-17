#include "UDPTransmitter.h"
#include <iostream>
#pragma comment(lib, "ws2_32.lib")
#include <ws2tcpip.h>
#include "Utility.h"

UDPTransmitter::UDPTransmitter(WAVEFORMATEX* pFormat, std::string destIp, unsigned short destPort) {
    packet.SamplingRate = pFormat->nSamplesPerSec;
    packet.Channels = pFormat->nChannels;
    packet.BitsPerSample = pFormat->wBitsPerSample;
    
    std::wstring wDestIp = to_wstring(destIp);
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    MustEquals(0, result, "WSAStartup");

    udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    CheckSocket(udpSocket, "socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)");

    memset(&destAddr, 0, sizeof(destAddr));
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(destPort);
    MustEquals(1, InetPton(AF_INET, wDestIp.c_str(), &destAddr.sin_addr), "InetPton()");
}

UDPTransmitter::~UDPTransmitter() {
    closesocket(udpSocket);
    WSACleanup();
}

void UDPTransmitter::bufferUpdateCallback(UINT64 u64DevicePosition, BYTE* data, UINT32 numFrames) {
    std::cout << "bufferUpdated()" << std::endl;

    packet.UpstreamDevicePosition = u64DevicePosition;
    packet.Frames = numFrames;
    packet.DataSize = numFrames * (packet.Channels * (packet.BitsPerSample / 8));
    packet.Data = data;

    int headerSize = sizeof(packet.SamplingRate) + sizeof(packet.Channels) + sizeof(packet.BitsPerSample)
        + sizeof(packet.UpstreamDevicePosition) + sizeof(packet.Frames) + sizeof(packet.DataSize);
    int totalSize = headerSize + packet.DataSize;

    buffer.resize(totalSize);

    BYTE* ptr = buffer.data();
    memcpy(ptr, &packet.SamplingRate, sizeof(packet.SamplingRate));                     ptr += sizeof(packet.SamplingRate);
    memcpy(ptr, &packet.Channels, sizeof(packet.Channels));                             ptr += sizeof(packet.Channels);
    memcpy(ptr, &packet.BitsPerSample, sizeof(packet.BitsPerSample));                   ptr += sizeof(packet.BitsPerSample);
    memcpy(ptr, &packet.UpstreamDevicePosition, sizeof(packet.UpstreamDevicePosition)); ptr += sizeof(packet.UpstreamDevicePosition);
    memcpy(ptr, &packet.Frames, sizeof(packet.Frames));                                 ptr += sizeof(packet.Frames);
    memcpy(ptr, &packet.DataSize, sizeof(packet.DataSize));                             ptr += sizeof(packet.DataSize);
    memcpy(ptr, packet.Data, packet.DataSize);

    std::cout << std::dec << "AudioPacket(SamplingRate=" << packet.SamplingRate << ", Channels=" << packet.Channels <<
        ", BitsPerSample=" << packet.BitsPerSample << ", UpstreamDevicePosition=" << packet.UpstreamDevicePosition <<
        ", Frames=" << packet.Frames << ", DataSize=" << packet.DataSize << ")" << std::endl;

    std::cout << "UDP packet sending" << std::endl;
    sendto(udpSocket, (const char*)buffer.data(), totalSize, 0, (struct sockaddr*)&destAddr, sizeof(destAddr));
    std::cout << "UDP packet sent" << std::endl;
}
