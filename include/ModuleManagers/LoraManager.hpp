#pragma once

#include "LoraUtilities.hpp"
#include "FilesystemUtils.h"
#include "LoraDriverInterface.h"
#include <atomic>

namespace
{
    const size_t  MAX_MESSAGE_SIZE         = 512;
    const size_t  AFTER_SEND_BLOCK_TIME_MS = 50;
    const size_t  NUM_REBROADCAST_ATTEMPTS = 1;
    const size_t  MIN_SEND_DELAY_MS        = 100;
    const size_t  MAX_SEND_DELAY_MS        = 3000;
    const uint8_t MAX_BOUNCES_LEFT         = 5;
}

namespace LoraModule
{

class Manager
{
public:
    static constexpr const char* TAG = "LoraManager";

    Manager(LoraDriverInterface* driver) : _Driver(driver) {}

    bool Init()
    {
        if (_Driver == nullptr) { return false; }
        if (!_Driver->Init())  { return false; }

        auto userID = FilesystemModule::Utilities::DeviceInfo().getUInt("UserID");
        LoraModule::Utilities::UserID() = userID;

        LoraModule::Utilities::Init();

        _sendQueue = System_Utils::getQueue(LoraModule::Utilities::MessageSendQueueID());
        if (_sendQueue == nullptr) { return false; }

        return true;
    }

    void RadioTask()
    {
        // Self-register task handle so the DIO0 ISR and SendQueueTask can notify us
        _ReceiveTaskHandle = xTaskGetCurrentTaskHandle();

        // Enter continuous receive mode — safe here because handle is now set
        _Driver->StartReceiving();

        while (true)
        {
            // Block until DIO0 ISR (packet received) or SendQueueTask (message to send) wakes us
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

            // Try to read a received packet (data already buffered by library ISR)
            uint8_t buffer[MAX_MESSAGE_SIZE];
            size_t  len = 0;

            if (_Driver->ReceiveMessage(buffer, len, 0))
            {
                auto msg = LoraModule::Utilities::DeserializeMessage(buffer, len);

                if (msg != nullptr)
                {
                    if (msg->bouncesLeft > MAX_BOUNCES_LEFT)
                    {
                        ESP_LOGW(TAG, "Clamping bouncesLeft %d -> %d from sender 0x%08X",
                                 msg->bouncesLeft, MAX_BOUNCES_LEFT, msg->sender);
                        msg->bouncesLeft = MAX_BOUNCES_LEFT;
                    }

                    if (msg->sender == LoraModule::Utilities::UserID())
                    {
                        ESP_LOGI(TAG, "Message echoed back from this node — dropping");
                        LoraModule::Utilities::IncrementEchoCount();
                    }
                    else
                    {
                        ESP_LOGI(TAG, "Received from sender 0x%08X  msgID 0x%08X  bouncesLeft %d",
                                 msg->sender, msg->msgID, msg->bouncesLeft);

                        bool shouldFwd = ShouldMessageBeForwarded(msg->sender, msg->msgID, msg->bouncesLeft);

                        if (!shouldFwd)
                        {
                            auto it = _lastReceivedMessages.find(msg->sender);
                            if (msg->bouncesLeft == 0)
                            {
                                ESP_LOGI(TAG, "Not forwarding — bouncesLeft == 0");
                            }
                            else if (it != _lastReceivedMessages.end() && it->second == msg->msgID)
                            {
                                ESP_LOGI(TAG, "Not forwarding — duplicate (last seen msgID 0x%08X)", it->second);
                            }
                        }

                        bool isNew = !LoraModule::Utilities::MessageExists(msg->sender, msg->msgID);
                        LoraModule::Utilities::RecordRouting(msg->sender, msg->msgID);

                        auto& events = LoraModule::Utilities::MessageEvents();
                        auto evIt = events.find(msg->SchemaGuid());
                        if (evIt != events.end())
                        {
                            evIt->second.Invoke(msg, isNew);
                        }

                        if (shouldFwd)
                        {
                            // Re-enter RX immediately so the radio stays live during the wait
                            _Driver->StartReceiving();

                            // RSSI-based backoff: weak signal = better relay candidate = shorter wait.
                            // Maps [-130, -80] dBm → [0, 2000] ms  (stronger signal = longer delay,
                            // letting distant nodes — which are better positioned — relay first).
                            int rssi = _Driver->PacketRssi();
                            int clampedRssi = rssi < -130 ? -130 : (rssi > -80 ? -80 : rssi);
                            uint32_t rssiDelayMs = static_cast<uint32_t>((clampedRssi + 130) * 2000 / 50);

                            ESP_LOGI(TAG, "Relay wait %u ms (RSSI %d dBm) — bouncesLeft %d -> %d",
                                     rssiDelayMs, rssi, msg->bouncesLeft, msg->bouncesLeft - 1);

                            if (rssiDelayMs > 0)
                            {
                                vTaskDelay(pdMS_TO_TICKS(rssiDelayMs));
                            }

                            msg->bouncesLeft--;
                            LoraModule::Utilities::SendMessage(msg);
                            _lastReceivedMessages[msg->sender] = msg->msgID;
                        }
                    }
                }
            }

            // Send any pending outbound message
            if (!_SendBufferIdle)
            {
                // Wait for a clear channel before transmitting
                while (_Driver->IsChannelBusy())
                {
                    ESP_LOGI(TAG, "Channel busy — waiting");
                    vTaskDelay(pdMS_TO_TICKS(AFTER_SEND_BLOCK_TIME_MS));
                }

                if (!_Driver->SendMessage(_SendBuffer, _SendBufferLen))
                {
                    ESP_LOGE(TAG, "Failed to send message");
                }
                _SendBufferIdle = true;
                vTaskDelay(pdMS_TO_TICKS(AFTER_SEND_BLOCK_TIME_MS));
            }

            // Re-enter continuous receive mode for the next packet
            _Driver->StartReceiving();
        }
    }

    void SendQueueTask()
    {
        if (_sendQueue == nullptr)
        {
            vTaskDelete(NULL);
        }

        while (true)
        {
            // Block until a message is queued
            std::shared_ptr<LoraModule::LoraMessageInterface>* wrapper = nullptr;
            if (xQueueReceive(_sendQueue, &wrapper, portMAX_DELAY) != pdTRUE) { continue; }

            auto msg = *wrapper;
            delete wrapper;

            bool isOwn = (msg->sender == LoraModule::Utilities::UserID());
            uint8_t attemptsLeft = isOwn
                ? std::max((uint8_t)1, LoraModule::Utilities::DefaultSendAttempts())
                : static_cast<uint8_t>(NUM_REBROADCAST_ATTEMPTS);

            ESP_LOGI(TAG, "Queued msgID 0x%08X sender 0x%08X — %s — attempts %d",
                     msg->msgID, msg->sender, isOwn ? "own" : "relay", attemptsLeft);

            while (attemptsLeft > 0)
            {
                // Random backoff in [MIN_SEND_DELAY_MS, MAX_SEND_DELAY_MS)
                uint32_t delayMs = MIN_SEND_DELAY_MS +
                    static_cast<uint32_t>(rand() % (MAX_SEND_DELAY_MS - MIN_SEND_DELAY_MS));
                vTaskDelay(pdMS_TO_TICKS(delayMs));

                // Wait for RadioTask to finish any in-progress send
                while (!_SendBufferIdle.load())
                {
                    vTaskDelay(pdMS_TO_TICKS(10));
                }

                size_t outLen = 0;
                if (LoraModule::Utilities::SerializeMessage(msg, _SendBuffer, outLen))
                {
                    _SendBufferLen = outLen;
                    _SendBufferIdle.store(false);

                    if (_ReceiveTaskHandle != nullptr)
                    {
                        xTaskNotifyGive(_ReceiveTaskHandle);
                    }

                    ESP_LOGI(TAG, "Transmitting msgID 0x%08X — attemptsLeft %d -> %d",
                             msg->msgID, attemptsLeft, attemptsLeft - 1);
                }
                else
                {
                    ESP_LOGE(TAG, "Failed to serialize msgID 0x%08X — dropping", msg->msgID);
                    break;
                }

                attemptsLeft--;
            }

            ESP_LOGI(TAG, "msgID 0x%08X — all attempts exhausted, removing", msg->msgID);
        }
    }

    void SetTaskHandles(TaskHandle_t sendHandle, TaskHandle_t receiveHandle)
    {
        _SendTaskHandle    = sendHandle;
        _ReceiveTaskHandle = receiveHandle;
    }

protected:
    bool ShouldMessageBeForwarded(uint32_t senderID, uint32_t msgID, uint8_t bouncesLeft)
    {
        if (senderID == LoraModule::Utilities::UserID()) { return false; }
        if (bouncesLeft == 0) { return false; }

        auto it = _lastReceivedMessages.find(senderID);
        if (it == _lastReceivedMessages.end()) { return true; }
        return it->second != msgID;
    }

    LoraDriverInterface* _Driver;

    QueueHandle_t _sendQueue = nullptr;

    std::unordered_map<uint32_t, uint32_t> _lastReceivedMessages;

    TaskHandle_t _SendTaskHandle    = nullptr;
    TaskHandle_t _ReceiveTaskHandle = nullptr;

    std::atomic<bool> _SendBufferIdle { true };
    uint8_t  _SendBuffer[MAX_MESSAGE_SIZE]{};
    size_t   _SendBufferLen = 0;
};

} // namespace LoraModule
