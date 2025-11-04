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
            Serial.println("Deinitializing any existing ESP-NOW instance");
            // esp_now_deinit();

            Serial.println("Enabling WiFi radio");
            RadioUtils::EnableRadio();

            Serial.println("Setting WiFi mode to WIFI_STA for ESP-NOW");
            WiFi.mode(WIFI_STA);

            if (esp_now_init() != ESP_OK)
            {
                #if DEBUG == 1
                Serial.println("Error initializing ESP-NOW");
                #endif
                return;
            }

            if (!Utilities::InitializeRpcQueue())
            {
                #if DEBUG == 1
                Serial.println("Error initializing RPC queue");
                #endif
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
                #if DEBUG == 1
                Serial.println("RPC queue is not initialized");
                #endif
                return false;
            }

            auto rpcQueueHandle = System_Utils::getQueue(rpcQueueID);

            if (rpcQueueHandle == nullptr)
            {
                #if DEBUG == 1
                Serial.println("Error getting RPC queue handle");
                #endif
                return false;
            }

            RpcData rpcRequestPacket;
            if (xQueueReceive(rpcQueueHandle, &rpcRequestPacket, 0) != pdTRUE)
            {
                #if DEBUG == 1
                // Serial.println("No RPC packet in queue");
                #endif
                return false;
            }

            #if DEBUG == 1
            Serial.println("Received RPC packet from queue");
            #endif

            auto err = deserializeMsgPack(doc, (const char *)rpcRequestPacket.data, rpcRequestPacket.length);

            if (err != DeserializationError::Ok)
            {
                Serial.printf("Error deserializing RPC packet: %d\n", err);
                return false;
            }

            #if DEBUG == 1
            Serial.println("Successfully deserialized RPC packet");
            serializeJsonPretty(doc, Serial);
            Serial.println();
            #endif
            
            return true;
        }

        bool SendData(const uint8_t *macAddress, const uint8_t *data, size_t len)
        {
            auto result = esp_now_send(macAddress, data, len);
            if (result != ESP_OK)
            {
                #if DEBUG == 1
                Serial.printf("Error sending data: %d\n", result);
                #endif
                return false;
            }

            return true;
        }
    };
}