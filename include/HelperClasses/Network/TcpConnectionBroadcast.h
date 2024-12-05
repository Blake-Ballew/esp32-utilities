#pragma once

#include "NetworkUtils.h"
#include "WiFiUdpStream.h"
#include <ArduinoJson.h>

namespace
{
    const int BROADCAST_PORT = 54789;
}

namespace NetworkModule
{
    // Broadcasts the IP address and port of a listening TCP server
    class TcpConnectionBroadcast : public WiFiUdpStream
    {
    public:
        TcpConnectionBroadcast() : WiFiUdpStream() {}

        ~TcpConnectionBroadcast() {}

        void SendBroadcast()
        {
            StaticJsonDocument<100> doc;
            doc["IpAddress"] = WiFi.localIP().toString();
            doc["Port"] = _portToBroadcast;

            if (!BeginPacket())
            {
                #if DEBUG == 1
                Serial.println("Failed to begin broadcast packet");
                #endif

                return;
            }
            else
            {
                #if DEBUG == 1
                // Serial.println("Broadcast packet started");
                #endif
            }

            if (serializeMsgPack(doc, GetStream()) == 0)
            {
                #if DEBUG == 1
                // Serial.println("Failed to serialize broadcast packet");
                #endif
            }
            else
            {
                #if DEBUG == 1
                // Serial.println("Broadcast packet serialized");
                #endif
            }

            if (!EndPacket())
            {
                #if DEBUG == 1
                // Serial.println("Failed to end broadcast packet");
                #endif
            }
            else
            {
                #if DEBUG == 1
                // Serial.println("Broadcast packet ended");
                #endif
            }

            Flush();
        }

        bool EstablishConnection(IPAddress ip, uint16_t port)
        {
            _remoteIP = ip;
            _remotePort = port;
            return _udp.begin(WiFi.localIP(), _remotePort) == 1;
            
        }

        bool BeginPacket() {
            return _udp.beginPacket(_remoteIP, _remotePort) == 1;
        }

        bool EndPacket() { 
            return _udp.endPacket() == 1;
        }

        // Gets and sets the port send in the packet
        void SetPortToBroadcast(int port) { _portToBroadcast = port; }
        int PortToBroadcast() { return _portToBroadcast; }

    protected:
        int _portToBroadcast = BROADCAST_PORT;
    };
}