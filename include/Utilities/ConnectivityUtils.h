#pragma once

#include <queue>
#include <memory>
#include <Arduino.h>
#include "System_Utils.h"
#include "esp_now.h"
// #include "WiFiManager.h"
// #include "AlooWifiManager.h"

namespace ConnectivityModule
{
    
    static const char *TAG = "ConnectivityModule";

    struct RpcData {
        uint8_t data[250];  // ESP-NOW max size
        size_t length;
    };

    enum WiFiProvisioningMode
    {
        WIFI_PROV_MODE_NONE = 0,
        WIFI_PROV_MODE_ESP_NOW = 1,
        WIFI_PROV_MODE_TEMP_AP = 2
    };

    class Utilities
    {
    public:
        static int& RpcQueueID()
        {
            static int queueID = -1;
            return queueID;
        }

        static int &RpcChannelID()
        {
            static int channelID = -1;
            return channelID;
        }

        // static WiFiManager &ProvisioningWiFiManager()
        // {
        //     // static WiFiManager wifiManager(WiFiProvisiningApSSID().c_str(), WiFiProvisiningApPassword().c_str());
        //     // return wifiManager;
        // }

        static WiFiProvisioningMode &ProvisioningMode()
        {
            static WiFiProvisioningMode mode = WIFI_PROV_MODE_NONE;
            return mode;
        }

        static std::string WiFiProvisiningApSSID()
        {
            auto ssid = System_Utils::DeviceName;;
            std::replace(ssid.begin(), ssid.end(), ' ', '_');
            return ssid;
        }

        // TODO: make this properly static and assigned at random
        static std::string WiFiProvisiningApPassword()
        {
            std::string deviceStr = std::to_string(System_Utils::DeviceID % 10000);
            return "setup" + std::string(4 - deviceStr.length(), '0') + deviceStr;
        }

        static bool InitializeRpcQueue()
        {
            if (RpcQueueID() == -1)
            {
                RpcQueueID() = System_Utils::registerQueue(3, sizeof(RpcData));
            }

            return RpcQueueID() != -1;
        }

        static void DeinitializeRpcQueue()
        {
            if (RpcQueueID() != -1)
            {
                System_Utils::deleteQueue(RpcQueueID());
                RpcQueueID() = -1;
            }
        }

        // Use this function as the data received callback if you're expecting an RPC packet
        static void DataReceivedRpc(const uint8_t * mac, const uint8_t *incomingData, int len)
        {
            if (len <= 0)
            {
                ESP_LOGW(TAG, "Received empty RPC packet");
                return;
            }

            RpcData data;

            if (len > sizeof(data.data))
            {
                len = sizeof(data.data);
            }
            // clear the data buffer
            memset(data.data, 0, sizeof(data.data));
            memcpy(data.data, incomingData, len);
            data.length = len;

            ESP_LOGD(TAG, "Received %d bytes of RPC data", len);

            std::string hexStr;
            for (int i = 0; i < len; i++)
            {
                char hex[4];
                snprintf(hex, sizeof(hex), "%02X ", data.data[i]);
                hexStr += hex;
            }
            ESP_LOGV(TAG, "RPC data: %s", hexStr.c_str());

            if (!System_Utils::sendToQueue(RpcQueueID(), &data, 0))
            {
                ESP_LOGE(TAG, "Error sending RPC data to queue");
            }
            else
            {
                ESP_LOGD(TAG, "RPC data sent to queue");
            }
        }

        static void InitializeWiFiProvisioning()
        {
            if (ProvisioningMode() == WIFI_PROV_MODE_ESP_NOW)
            {
                ESP_LOGI(TAG, "Initializing ESP-NOW for WiFi Provisioning");
                InitializeEspNow().Invoke(DataReceivedRpc, nullptr);
            }
            else if (ProvisioningMode() == WIFI_PROV_MODE_TEMP_AP)
            {
                // ProvisioningWiFiManager().setConfigPortalBlocking(false);
                // ProvisioningWiFiManager().autoConnect(WiFiProvisiningApSSID().c_str(), WiFiProvisiningApPassword().c_str());
                // ProvisioningWiFiManager().begin(true);
            }
        }

        static void DeinitializeWiFiProvisioning()
        {
            if (ProvisioningMode() == WIFI_PROV_MODE_ESP_NOW)
            {
                DeinitializeEspNow().Invoke(false);
            }
            else if (ProvisioningMode() == WIFI_PROV_MODE_TEMP_AP)
            {
                // ProvisioningWiFiManager().
                // WiFiManager().stopConfigPortal();
            }
        }

        static EventHandler<esp_now_recv_cb_t, esp_now_send_cb_t> &InitializeEspNow()
        {
            static EventHandler<esp_now_recv_cb_t, esp_now_send_cb_t> espNowEventHandler;
            return espNowEventHandler;
        }

        static EventHandler<bool> &DeinitializeEspNow()
        {
            static EventHandler<bool> espNowDeinitEventHandler;
            return espNowDeinitEventHandler;
        }

        static void ProcessSettings(JsonDocument &doc)
        {
            if (doc.containsKey("WiFi Provisioning"))
            {
                auto mode = doc["WiFi Provisioning"]["cfgVal"].as<int>();
                if (mode >= WIFI_PROV_MODE_NONE && mode <= WIFI_PROV_MODE_TEMP_AP)
                {
                    ProvisioningMode() = static_cast<WiFiProvisioningMode>(mode);
                }
                else
                {
                    ProvisioningMode() = WIFI_PROV_MODE_NONE;
                }
            }
        }
    };
}