#pragma once

#include "System_Utils.h"
#include "NetworkStreamInterface.h"

#include <ArduinoJson.h>
#include <unordered_map>
#include <WiFi.h>
#include <Client.h>

// Reponsible for UDP and TCP connection over WiFi and eventually Ethernet
namespace NetworkModule
{
    namespace
    {
        const char *NETWORK_CONFIG_FILENAME PROGMEM = "/NetworkConfig.msgpk";

        const char* WIFI_SSID_DEFAULT PROGMEM = "Rum Ham";
        const char* WIFI_PASSWORD_DEFAULT PROGMEM = "Wildcard!";

        std::string _WifiSSID = WIFI_SSID_DEFAULT;
        std::string _WifiPassword = WIFI_PASSWORD_DEFAULT;

        int _RdpTaskID = -1;

        std::unordered_map<int, NetworkStreamInterface *> _NetworkStreams;
        int _NextNetworkStreamID = 0;

        // Map RPC channel IDs to NetworkStream IDs
        std::unordered_map<int, int> _RpcStreams;
    }

    class Utilities
    {
    public:

        const static int RPC_PORT = 14589;

        static bool EnableWiFi()
        {
            #if DEBUG == 1
            Serial.println("Enabling WiFi");
            #endif
            
            System_Utils::enableRadio();
            WiFi.disconnect(false);  // Reconnect the network
            WiFi.mode(WIFI_STA);    // Switch WiFi on
        
            WiFi.begin(_WifiSSID.c_str(), _WifiPassword.c_str());
        
            size_t timeoutCounter = 0;
            while (WiFi.status() != WL_CONNECTED) {
                vTaskDelay(pdMS_TO_TICKS(500));
                if (timeoutCounter++ > 20)
                {
                    return false;
                }
            }

            #if DEBUG == 1
            Serial.println("WiFi connected");
            Serial.println("IP address: ");
            Serial.println(WiFi.localIP());
            #endif
            return true;
        }

        // TODO: How to handle active streams?
        static void DisableWiFi()
        {
            System_Utils::disableRadio();
            WiFi.disconnect(true);  // Disconnect from the network
            WiFi.mode(WIFI_OFF);    // Switch WiFi off
        }

        static IPAddress GetBroadcastIP()
        {
            return WiFi.broadcastIP();
        }

        static IPAddress GetLocalIP()
        {
            return System_Utils::getLocalIP();
        }

        static void GetWiFiConfig(JsonDocument &doc)
        {
            doc["ssid"] = _WifiSSID;
            doc["password"] = _WifiPassword;
        }

        static int RegisterNetworkStream(NetworkStreamInterface *stream)
        {
            _NetworkStreams[_NextNetworkStreamID] = stream;
            return _NextNetworkStreamID++;
        }

        static void UnregisterNetworkStream(int id)
        {
            _NetworkStreams.erase(id);
        }

        static NetworkStreamInterface *GetNetworkStream(int id)
        {
            return _NetworkStreams[id];
        }

        static void SetWiFiConfig(JsonDocument &doc)
        {
            if (doc.containsKey("ssid") && doc.containsKey("password"))
            {
                _WifiSSID = doc["ssid"].as<std::string>();
                _WifiPassword = doc["password"].as<std::string>();
            }
        }

        static bool RpcRequestHandler(int channelID, JsonDocument &payload)
        {

            if (_RpcStreams.find(channelID) != _RpcStreams.end() &&
                _NetworkStreams.find(_RpcStreams[channelID]) != _NetworkStreams.end())
            {
                auto networkStreamID = _RpcStreams[channelID];
                auto result = deserializeMsgPack(payload, _NetworkStreams[networkStreamID]->GetStream());

                if (result == DeserializationError::Ok)
                {
                    return true;
                }
            }

            return false;
        }

        static void RpcResponseHandler(int channelID, JsonDocument &payload)
        {
            if (_RpcStreams.find(channelID) != _RpcStreams.end() &&
                _NetworkStreams.find(_RpcStreams[channelID]) != _NetworkStreams.end())
            {
                auto networkStreamID = _RpcStreams[channelID];
                auto stream = _NetworkStreams[networkStreamID];

                stream->BeginPacket();
                serializeMsgPack(payload, stream->GetStream());
                stream->EndPacket();
            }
        }

        static void AttachRpcStream(int rpcChannelID, int networkStreamID)
        {
            if (_RpcStreams.find(rpcChannelID) == _RpcStreams.end() &&
                _NetworkStreams.find(networkStreamID) != _NetworkStreams.end())
            {
                _RpcStreams[rpcChannelID] = networkStreamID;
            }
        }

        static void DetachRpcStream(int rpcChannelID)
        {
            if (_RpcStreams.find(rpcChannelID) != _RpcStreams.end())
            {
                _RpcStreams.erase(rpcChannelID);
            }
        }
    };
}