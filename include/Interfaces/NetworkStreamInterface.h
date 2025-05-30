#pragma once

#include <Arduino.h>

namespace ConnectivityModule
{
    // Reponsible for UDP and TCP connection over WiFi or Ethernet
    // Connection is automatically closed when the object is destroyed
    class NetworkStreamInterface
    {
    public:
        NetworkStreamInterface() {}
        NetworkStreamInterface(uint16_t port) : _port(port) {}
        virtual ~NetworkStreamInterface() {}

        virtual Stream &GetStream() = 0;
        uint16_t Port() { return _port; }

        void SetPort(uint16_t port) { _port = port; }

        virtual bool BeginPacket() = 0;
        virtual bool EndPacket() = 0;
        virtual void Flush() = 0;

    protected:
        uint16_t _port = 0;
    };
}