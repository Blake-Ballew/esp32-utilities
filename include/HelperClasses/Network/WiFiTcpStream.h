#pragma once

#include "NetworkStreamInterface.h"
#include <WiFi.h>

namespace ConnectivityModule
{
    class WiFiTcpStream : public NetworkStreamInterface
    {
    public:
        WiFiTcpStream() : NetworkStreamInterface() {}
        WiFiTcpStream(uint16_t port) : NetworkStreamInterface(port) {}

        ~WiFiTcpStream()
        {
            if (_client.connected()) {
                _client.stop();
            }
        }

        // Starts server for a fixed amount of time
        bool EstablishServerConnection(
            size_t timeoutMs = 10000,
            uint8_t maxConnections = 1)
        {
            #if DEBUG == 1
            Serial.print("Starting server on port ");
            Serial.println(_port);
            #endif

            _server = WiFiServer(_port, maxConnections);
            _server.begin(_port);
            auto startTime = xTaskGetTickCount();

            do {
                if (_client = _server.available()) {
                    break;
                }
                vTaskDelay(1);
            } while ((xTaskGetTickCount() - startTime) < timeoutMs);

            _server.stop();
            return _client.connected();
        }

        // Starts server in background
        void StartServer(uint8_t maxConnections = 1) 
        {
            #if DEBUG == 1
            Serial.print("Starting server on port ");
            Serial.println(_port);
            #endif
            _server = WiFiServer(_port, maxConnections);
            _server.begin(_port);
        }

        bool AcceptClientConnection()
        {
            _client = _server.available();
            if (_client.connected())
            {
                _server.stop();
                return true;
            }

            return false;
        }

        void StopServer() { _server.stop(); }

        bool EstablishClientConnection(
            IPAddress ip, 
            size_t timeoutMs = 10000)
        {
            if (!_client.connect(ip, _port, timeoutMs)) 
            {
                return false;
            }

            return true;
        }

        Stream &GetStream() { return _client; }

        bool BeginPacket() { return true; }

        bool EndPacket() { return true; }

        void Flush() { _client.flush(); }

        bool IsConnected() { return _client.connected(); }

        WiFiClient &GetClient() { return _client; }

    protected:
        WiFiClient _client;
        WiFiServer _server;
    };
}