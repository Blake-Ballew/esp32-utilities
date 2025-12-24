#include "RadioUtils.h"
#include <queue>
#include <memory>
#include <esp_now.h>
#include "WiFi.h"
#include "ConnectivityUtils.h"

namespace ConnectivityModule
{

    class EspNowManager
    {
    private:
        static constexpr const char *TAG = "EspNowManager";

    public:
        EspNowManager() {}
        ~EspNowManager() {}

        // Radio channel to use for ESP-NOW
        int &RadioChannel()
        {
            static int channel = 0;
            return channel;
        }

        void Initialize(
            esp_now_recv_cb_t dataReceivedCallback = nullptr,
            esp_now_send_cb_t dataSentCallback = nullptr)
        {
            // Ensure ESP-NOW is deinitialized before initializing
            ESP_LOGI(TAG, "Deinitializing any existing ESP-NOW instance");
            // esp_now_deinit();

            ESP_LOGI(TAG, "Enabling WiFi radio");
            RadioUtils::EnableRadio();

            ESP_LOGI(TAG, "Setting WiFi mode to WIFI_STA for ESP-NOW");
            WiFi.mode(WIFI_STA);

            if (esp_now_init() != ESP_OK)
            {
                ESP_LOGE(TAG, "Error initializing ESP-NOW");
                return;
            }

            if (!Utilities::InitializeRpcQueue())
            {
                ESP_LOGE(TAG, "Error initializing RPC queue");
                return;
            }
            
            if (dataReceivedCallback != nullptr)
            {
                esp_now_register_recv_cb(dataReceivedCallback);
            }

            if (dataSentCallback != nullptr)
            {
                esp_now_register_send_cb(dataSentCallback);
            }

            esp_wifi_set_channel(RadioChannel(), WIFI_SECOND_CHAN_NONE);
        }

        void Deinitialize(bool disableRadio = true)
        {
            esp_now_deinit();

            if (disableRadio)
            {
                RadioUtils::DisableRadio();
            }

            Utilities::DeinitializeRpcQueue();
        }

        // Feed this function into the RPC manager as a request source to receive RPC packets over ESP-NOW
        bool ReceiveRpcQueue(int _, JsonDocument &doc)
        {
            auto rpcQueueID = Utilities::RpcQueueID();

            if (rpcQueueID == -1)
            {
                ESP_LOGW(TAG, "RPC queue is not initialized");
                return false;
            }

            auto rpcQueueHandle = System_Utils::getQueue(rpcQueueID);

            if (rpcQueueHandle == nullptr)
            {
                ESP_LOGE(TAG, "Error getting RPC queue handle");
                return false;
            }

            RpcData rpcRequestPacket;
            if (xQueueReceive(rpcQueueHandle, &rpcRequestPacket, 0) != pdTRUE)
            {
                ESP_LOGV(TAG, "No RPC packet in queue");
                return false;
            }

            ESP_LOGD(TAG, "Received RPC packet from queue");

            auto err = deserializeMsgPack(doc, (const char *)rpcRequestPacket.data, rpcRequestPacket.length);

            if (err != DeserializationError::Ok)
            {
                ESP_LOGE(TAG, "Error deserializing RPC packet: %d", err);
                return false;
            }

            std::string buf;
            serializeJsonPretty(doc, buf);
            ESP_LOGV(TAG, "Successfully deserialized RPC packet: %s", buf.c_str());

            return true;
        }

        bool SendData(const uint8_t *macAddress, const uint8_t *data, size_t len)
        {
            auto result = esp_now_send(macAddress, data, len);
            if (result != ESP_OK)
            {
                ESP_LOGE(TAG, "Error sending data: %d", result);
                return false;
            }

            return true;
        }
    };
}