#pragma once

#include "NetworkStreamInterface.h"
#include <WiFi.h>
#include <WiFiUdp.h>

namespace ConnectivityModule
{
    class WiFiUdpStream : public NetworkStreamInterface
    {
    public:
        WiFiUdpStream() : NetworkStreamInterface() {}
        WiFiUdpStream(uint16_t port) : NetworkStreamInterface(port) {}

        ~WiFiUdpStream() { 
            _udp.stop();
        }

        Stream &GetStream() { return _udp; }

        bool BeginPacket() { return _udp.beginPacket(_remoteIP, _remotePort) == 1; }

        bool EndPacket() { return _udp.endPacket() == 1; }

        void Flush() { _udp.flush(); }

        bool EstablishConnection(IPAddress ip, uint16_t port) 
        { 
            #if DEBUG == 1
            Serial.print("Connecting to: ");
            Serial.print(ip);
            Serial.print(":");
            Serial.println(port);
            #endif
            _remoteIP = ip;
            _remotePort = port;
            return _udp.begin(WiFi.localIP(), port) == 1; 
        }

    protected:
        WiFiUDP _udp;

        IPAddress _remoteIP = IPAddress(0, 0, 0, 0);
        uint16_t _remotePort = 0;
    };
}
