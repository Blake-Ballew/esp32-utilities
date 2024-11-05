#pragma once

#include "NetworkStreamInterface.h"
#include <WiFi.h>
#include <WiFiUdp.h>

namespace NetworkModule
{
    class WiFiUdpStream : public NetworkStreamInterface
    {
    public:
        WiFiUdpStream(uint16_t port) : NetworkStreamInterface(port) {}

        ~WiFiUdpStream() { 
            _udp.stop();
        }

        Stream &GetStream() { return _udp; }

        void BeginPacket() { _udp.beginPacket(); }

        void EndPacket() { _udp.endPacket(); }

        void Flush() { _udp.flush(); }

        bool EstablishConnection(IPAddress ip, uint16_t port) { return _udp.begin(ip, port); }

    protected:
        WiFiUDP _udp;
    };
}
