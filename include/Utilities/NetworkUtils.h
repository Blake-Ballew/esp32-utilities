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

        const char* WIFI_SSID_DEFAULT PROGMEM = "ESP32-OTA";
        const char* WIFI_PASSWORD_DEFAULT PROGMEM = "e65v41ev";

        std::string _WifiSSID = WIFI_SSID_DEFAULT;
        std::string _WifiPassword = WIFI_PASSWORD_DEFAULT;

        int _RdpTaskID = -1;

        std::unordered_map<int, NetworkStreamInterface *> _NetworkStreams;
        int _NextNetworkStreamID = 0;
    }

    class Utilities
    {
    public:
        static bool EnableWiFi()
        {
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
        }

        static void DisableWiFi()
        {
            System_Utils::enableRadio();
            WiFi.disconnect(true);  // Disconnect from the network
            WiFi.mode(WIFI_OFF);    // Switch WiFi off
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
            delete _NetworkStreams[id];
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

        static void StartRpcTask()
        {
            if (_RdpTaskID == -1)
            {
                _RdpTaskID = System_Utils::registerTask(RpcTask, "Rpc Task", 4096, NULL, 1, 1);
            }

            vTaskResume(System_Utils::getTask(_RdpTaskID));
        }

        static void StopRpcTask()
        {
            if (_RdpTaskID != -1)
            {
                vTaskSuspend(System_Utils::getTask(_RdpTaskID));
            }
        }

        static void RpcTask(void *pvParameters)
        {
            while (true)
            {
                vTaskDelay(pdMS_TO_TICKS(500));

                
            }
        }
    };
}