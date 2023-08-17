#include "UDPReceiver.h"
#include <chrono>
#include <ctime>
#include "Utility.h"

UDPReceiver::UDPReceiver(unsigned short listenPort)
    : listening(false),
    buffer(new char[BUFFER_SIZE]) {

    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    MustEquals(0, result, "WSAStartup");

    udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    CheckSocket(udpSocket, "socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)");

    memset(&listenAddr, 0, sizeof(listenAddr));
    listenAddr.sin_family = AF_INET;
    listenAddr.sin_port = htons(listenPort);
    listenAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    DWORD timeout = 1000;
    result = setsockopt(udpSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
    MustNotEquals(SOCKET_ERROR, result, "setsockopt()");

    result = bind(udpSocket, (struct sockaddr*)&listenAddr, sizeof(listenAddr));
    MustNotEquals(SOCKET_ERROR, result, "bind()");
}

void UDPReceiver::startListening() {
    listening = true;
    listenThread = std::thread(&UDPReceiver::listenLoop, this);
}

void UDPReceiver::stopListening() {
    listening = false;
    if (listenThread.joinable()) {
        listenThread.join();
    }
}

bool UDPReceiver::DecodeAudioPacket(const char* data, int dataSize, AudioPacket& packet) {
    if (!data || dataSize <= 0) {
        return false;
    }

    const BYTE* ptr = reinterpret_cast<const BYTE*>(data);

    int headerSize = sizeof(packet.SamplingRate) + sizeof(packet.Channels) + sizeof(packet.BitsPerSample)
        + sizeof(packet.UpstreamDevicePosition) + sizeof(packet.Frames) + sizeof(packet.DataSize);
    if (dataSize < headerSize) {
        return false;
    }

    memcpy(&packet.SamplingRate, ptr, sizeof(packet.SamplingRate));                     ptr += sizeof(packet.SamplingRate);
    memcpy(&packet.Channels, ptr, sizeof(packet.Channels));                             ptr += sizeof(packet.Channels);
    memcpy(&packet.BitsPerSample, ptr, sizeof(packet.BitsPerSample));                   ptr += sizeof(packet.BitsPerSample);
    memcpy(&packet.UpstreamDevicePosition, ptr, sizeof(packet.UpstreamDevicePosition)); ptr += sizeof(packet.UpstreamDevicePosition);
    memcpy(&packet.Frames, ptr, sizeof(packet.Frames));                                 ptr += sizeof(packet.Frames);
    memcpy(&packet.DataSize, ptr, sizeof(packet.DataSize));                             ptr += sizeof(packet.DataSize);

    if (dataSize < (headerSize + packet.DataSize)) {
        return false;
    }

    packet.Data = new BYTE[packet.DataSize];
    memcpy(packet.Data, ptr, packet.DataSize);

    return true;
}

void UDPReceiver::listenLoop() {
    sockaddr_in from{};
    int fromSize = sizeof(from);
    int receivedBytes;
    int error;

    AudioPacket packet{};
    while (listening) {
        receivedBytes = recvfrom(udpSocket, buffer.get(), BUFFER_SIZE, 0, (struct sockaddr*)&from, &fromSize);

        if (receivedBytes == SOCKET_ERROR) {
            error = WSAGetLastError();

            auto now = std::chrono::system_clock::now();
            std::time_t current_time = std::chrono::system_clock::to_time_t(now);

            char timeString[26]; 
            errno_t err = ctime_s(timeString, sizeof(timeString), &current_time);
            std::cout << timeString;

            if (error == WSAETIMEDOUT) {
                std::cerr << "No packet arrived." << std::endl;
                continue;
            }
            std::cerr << "WSAGetLastError() returned " << error << std::endl;
            continue;
        }
        std::cout << "Received " << receivedBytes << " bytes." << std::endl;
        bool success = DecodeAudioPacket(buffer.get(), receivedBytes, packet);
        MustEquals(1, success, "DecodeAudioPacket()");

        std::cout << std::dec << "AudioPacket(SamplingRate=" << packet.SamplingRate << ", Channels=" << packet.Channels <<
            ", BitsPerSample=" << packet.BitsPerSample << ", UpstreamDevicePosition=" << packet.UpstreamDevicePosition <<
            ", Frames=" << packet.Frames << ", DataSize=" << packet.DataSize << ")" << std::endl;           
    }
}

UDPReceiver::~UDPReceiver() {
    if (listening) {
        stopListening();
    }
    closesocket(udpSocket);
    WSACleanup();
}
