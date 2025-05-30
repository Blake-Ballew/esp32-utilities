// #pragma once

// #include "System_Utils.h"
// #include "NetworkStreamInterface.h"
// #include <esp_now.h>
// #include <ArduinoJson.h>
// #include <unordered_map>
// #include <WiFi.h>
// #include <Client.h>

// #include <ESPAsyncWebServer.h>

// // 

// // Reponsible for UDP and TCP connection over WiFi and eventually Ethernet
// namespace NetworkModule
// {
//     namespace
//     {
//         const char *NETWORK_CONFIG_FILENAME PROGMEM = "/NetworkConfig.msgpk";

//         const char* WIFI_SSID_DEFAULT PROGMEM = "Rum Ham";
//         const char* WIFI_PASSWORD_DEFAULT PROGMEM = "Wildcard!";

//         std::string _WifiSSID = WIFI_SSID_DEFAULT;
//         std::string _WifiPassword = WIFI_PASSWORD_DEFAULT;

//         std::string _WiFiApSSID = "ESP32-Utilities-AP";
//         std::string _WiFiApPassword = "esp-ap-password";

//         int _RdpTaskID = -1;

//         std::unordered_map<int, NetworkStreamInterface *> _NetworkStreams;
//         int _NextNetworkStreamID = 0;

//         // Map RPC channel IDs to NetworkStream IDs
//         std::unordered_map<int, int> _RpcStreams;

//         AsyncWebServer _WebServer(80);

//         int _WiFiChannel = 11;

//         bool _WiFiEnabled = false;
//         bool _EspNowEnabled = false;
//     }

//     // enum
//     // {
//     //     WIFI_RADIO_OFF = 0,
//     //     WIFI_STATION,
//     //     WIFI_ACCESS_POINT,
//     //     WIFI_AWARE,
//     //     // ESP_NOW
//     // } WiFiRadioOperationMode;

//     class Utilities
//     {
//     public:

//         const static int RPC_PORT = 14589;

//         static bool EnableWiFi()
//         {
//             #if DEBUG == 1
//             Serial.println("Enabling WiFi");
//             #endif
            
//             System_Utils::enableRadio();
//             WiFi.disconnect(false);  // Reconnect the network
//             WiFi.mode(WIFI_STA);    // Switch WiFi on
        
//             WiFi.begin(_WifiSSID.c_str(), _WifiPassword.c_str());
        
//             size_t timeoutCounter = 0;
//             while (WiFi.status() != WL_CONNECTED) {
//                 vTaskDelay(pdMS_TO_TICKS(500));
//                 if (timeoutCounter++ > 20)
//                 {
//                     return false;
//                 }
//             }

//             #if DEBUG == 1
//             Serial.println("WiFi connected");
//             Serial.println("IP address: ");
//             Serial.println(WiFi.localIP());
//             #endif
//             return true;
//         }

//         // TODO: How to handle active streams?
//         static void DisableWiFi()
//         {
//             System_Utils::disableRadio();
//             WiFi.disconnect(true);  // Disconnect from the network
//             WiFi.mode(WIFI_OFF);    // Switch WiFi off
//         }

//         // Turns the radio on if needed and starts an access point
//         static bool EnableWiFiAP()
//         {
//             auto wifiMode = WiFi.getMode();

//             if (wifiMode == WIFI_OFF)
//             {
//                 EnableRadio();
//             }

//             _WiFiEnabled = WiFi.softAP(_WiFiApSSID.c_str(), _WiFiApPassword.c_str(), _WiFiChannel);

//             return _WiFiEnabled;
//         }

//         static void DisableWiFiAP()
//         {
//             WiFi.softAPdisconnect();

//             if (!_EspNowEnabled)
//             {
//                 DisableRadio();
//             }
//         }

//         static void EnableEspNow()
//         {
//             if (_EspNowEnabled) return;
//             if (WiFi.getMode() == WIFI_OFF) EnableRadio();

//             auto returnCode = esp_now_init();

//             if (returnCode != ESP_OK)
//             {
//                 #if DEBUG == 1
//                 Serial.print("esp_now_init failed with error code: 0x");
//                 Serial.println(returnCode, HEX);
//                 #endif
//                 _EspNowEnabled = false;
//                 if (!_WiFiEnabled) DisableRadio();
//             }
//             else
//             {
//                 _EspNowEnabled = true;
//             }
//         }

//         static void DisableEspNow()
//         {
//             esp_now_deinit();
//             if (!_WiFiEnabled) DisableRadio();
//             _EspNowEnabled = false;
//         }

//         static void EnableRadio()
//         {
//             WiFi.mode(WIFI_AP);

//             // Janky way to change radio channel even if only using esp now
//             if (!_WiFiEnabled)
//             {
//                 WiFi.softAP(_WiFiApSSID.c_str(), _WiFiApPassword.c_str(), _WiFiChannel);
//                 WiFi.softAPdisconnect();
//             }
//         }

//         static void DisableRadio()
//         {
//             WiFi.mode(WIFI_OFF);
//         }

//         static IPAddress GetBroadcastIP()
//         {
//             return WiFi.broadcastIP();
//         }

//         static IPAddress GetLocalIP()
//         {
//             return System_Utils::getLocalIP();
//         }

//         static void GetWiFiConfig(JsonDocument &doc)
//         {
//             doc["ssid"] = _WifiSSID;
//             doc["password"] = _WifiPassword;
//         }

//         static int RegisterNetworkStream(NetworkStreamInterface *stream)
//         {
//             _NetworkStreams[_NextNetworkStreamID] = stream;
//             return _NextNetworkStreamID++;
//         }

//         static void UnregisterNetworkStream(int id)
//         {
//             if (_NetworkStreams.find(id) != _NetworkStreams.end())
//             {
//                 _NetworkStreams.erase(id);
//             }
//         }

//         static NetworkStreamInterface *GetNetworkStream(int id)
//         {
//             return _NetworkStreams[id];
//         }

//         static void SetWiFiConfig(JsonDocument &doc)
//         {
//             if (doc.containsKey("ssid") && doc.containsKey("password"))
//             {
//                 _WifiSSID = doc["ssid"].as<std::string>();
//                 _WifiPassword = doc["password"].as<std::string>();
//             }
//         }

//         static bool RpcRequestHandler(int channelID, JsonDocument &payload)
//         {
//             #if DEBUG == 1
//             // Serial.print("NetworkUtils::RpcRequestHandler. channelID: "); 
//             // Serial.println(channelID);
//             #endif

//             if (_RpcStreams.find(channelID) != _RpcStreams.end() &&
//                 _NetworkStreams.find(_RpcStreams[channelID]) != _NetworkStreams.end())
//             {
//                 auto networkStreamID = _RpcStreams[channelID];
//                 auto result = deserializeMsgPack(payload, _NetworkStreams[networkStreamID]->GetStream());

//                 if (result == DeserializationError::Ok)
//                 {
//                     #if DEBUG == 1
//                     Serial.println("NetworkUtils::RpcRequestHandler. result: Ok");
//                     #endif
//                     return true;
//                 }
//             }

//             return false;
//         }

//         static void RpcResponseHandler(int channelID, JsonDocument &payload)
//         {
//             if (_RpcStreams.find(channelID) != _RpcStreams.end() &&
//                 _NetworkStreams.find(_RpcStreams[channelID]) != _NetworkStreams.end())
//             {
//                 auto networkStreamID = _RpcStreams[channelID];
//                 auto stream = _NetworkStreams[networkStreamID];

//                 #if DEBUG == 1
//                 Serial.println("NetworkUtils::RpcResponseHandler. sening payload: ");
//                 serializeJson(payload, Serial);
//                 Serial.println("");
//                 #endif

//                 stream->BeginPacket();
//                 auto result = serializeMsgPack(payload, stream->GetStream());
//                 #if DEBUG == 1
//                 Serial.print("NetworkUtils::RpcResponseHandler. bytes written: ");
//                 Serial.println(result);
//                 Serial.println("Bytes: ");
//                 uint8_t buffer[measureMsgPack(payload)];
//                 serializeMsgPack(payload, buffer, sizeof(buffer));
//                 for (int i = 0; i < result; i++)
//                 {
//                     Serial.print(buffer[i], HEX);
//                     Serial.print(" ");
//                 }
//                 Serial.println("");
//                 #endif
//                 stream->EndPacket();
//             }
//         }

//         static void AttachRpcStream(int rpcChannelID, int networkStreamID)
//         {
//             if (_RpcStreams.find(rpcChannelID) == _RpcStreams.end() &&
//                 _NetworkStreams.find(networkStreamID) != _NetworkStreams.end())
//             {
//                 _RpcStreams[rpcChannelID] = networkStreamID;
//             }
//         }

//         static void DetachRpcStream(int rpcChannelID)
//         {
//             if (_RpcStreams.find(rpcChannelID) != _RpcStreams.end())
//             {
//                 _RpcStreams.erase(rpcChannelID);
//             }
//         }

//         static AsyncWebServer &WebServer() { return _WebServer; }

//         static bool WifiEnabled() { 
//             // Check if radio is on
//             if (WiFi.getMode() == WIFI_OFF)
//             {
//                 return false;
//             }

//             return WiFi.status() == WL_CONNECTED; }

        
//     };
// }