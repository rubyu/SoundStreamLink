#pragma once

#include "AudioPacket.h"

class IUDPReceiverBufferPreparedEventListener {
public:
    virtual ~IUDPReceiverBufferPreparedEventListener() = default;
    virtual void UDPReceiverBufferPreparedCallback(AudioPacket* packet) = 0;
};
