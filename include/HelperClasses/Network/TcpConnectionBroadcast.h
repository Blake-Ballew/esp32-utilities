#pragma once

#include "NetworkUtils.h"
#include "WiFiUdpStream.h"
#include <ArduinoJson.h>
#define LOG_TAG __FILE__

namespace
{
    const int BROADCAST_PORT = 54789;
}

namespace ConnectivityModule
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
                ESP_LOGE(TAG, "Failed to begin broadcast packet");
                return;
            }

            if (serializeMsgPack(doc, GetStream()) == 0)
            {
                ESP_LOGV(TAG, "Failed to serialize broadcast packet");
            }

            if (!EndPacket())
            {
                ESP_LOGV(TAG, "Failed to end broadcast packet");
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