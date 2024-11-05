#pragma once

#include "NetworkStreamInterface.h"
#include <WiFi.h>

namespace NetworkModule
{
    class WiFiTcpStream : public NetworkStreamInterface
    {
    public:
        WiFiTcpStream(uint16_t port) : NetworkStreamInterface(port) {}

        ~WiFiTcpStream()
        {
            if (_client.connected()) {
                _client.stop();
            }
        }

        bool EstablishServerConnection(
            size_t timeoutMs = 10000,
            uint8_t maxConnections = 1)
        {
            WiFiServer server(_port, maxConnections);

            server.begin();
            auto startTime = xTaskGetTickCount();

            do {
                if (_client = server.available()) {
                    break;
                }
                vTaskDelay(1);
            } while ((xTaskGetTickCount() - startTime) < timeoutMs);

            return _client.connected();
        }

        bool EstablishClientConnection(
            IPAddress ip, 
            size_t timeoutMs = 10000)
        {
            if (!_client.connect(ip, _port, timeoutMs)) {
                return false;
            }

            return true;
        }

        Stream &GetStream() { return _client; }

        void BeginPacket() {}

        void EndPacket() {}

        void Flush() { _client.flush(); }

        bool IsConnected() { return _client.connected(); }

    protected:
        WiFiClient _client;
        
    };
}