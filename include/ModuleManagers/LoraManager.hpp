#pragma once

#include "LoraUtilities.hpp"
#include "FilesystemUtils.h"
#include "LoraDriverInterface.h"

namespace
{
    const size_t MAX_MESSAGE_SIZE            = 512;
    const uint8_t DEFAULT_NODE_ID            = 1;
    const size_t MESSAGE_RECEIVE_TIMEOUT_MS  = 100;
    const size_t RECEIVE_THREAD_SLEEP_MS     = 100;
    const size_t AFTER_SEND_BLOCK_TIME_MS    = 500;
    const size_t NUM_REBROADCAST_ATTEMPTS    = 1;
    const size_t SEND_THREAD_TICK_MS         = 250;
    const size_t  MAX_SEND_DELAY_TICKS       = 15;
    const uint8_t MAX_BOUNCES_LEFT           = 5;
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
        while (true)
        {
            uint8_t buffer[MAX_MESSAGE_SIZE];
            size_t  len = 0;

            if (_Driver->ReceiveMessage(buffer, len, MESSAGE_RECEIVE_TIMEOUT_MS))
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
                            ESP_LOGI(TAG, "Forwarding — bouncesLeft %d -> %d", msg->bouncesLeft, msg->bouncesLeft - 1);
                            msg->bouncesLeft--;
                            LoraModule::Utilities::SendMessage(msg);
                            _lastReceivedMessages[msg->sender] = msg->msgID;
                        }
                    }
                }
            }

            if (!_SendBufferIdle)
            {
                if (!_Driver->SendMessage(_SendBuffer, _SendBufferLen))
                {
                    ESP_LOGE(TAG, "Failed to send message");
                }
                _SendBufferIdle = true;
                vTaskDelay(pdMS_TO_TICKS(AFTER_SEND_BLOCK_TIME_MS));
                continue;
            }

            vTaskDelay(pdMS_TO_TICKS(RECEIVE_THREAD_SLEEP_MS));
        }
    }

    void SendQueueTask()
    {
        if (_sendQueue == nullptr)
        {
            vTaskDelete(NULL);
        }

        struct QueuedMsg
        {
            std::shared_ptr<LoraModule::LoraMessageInterface> msg;
            uint8_t attemptsLeft;
            size_t  ticksLeft;
        };

        std::vector<QueuedMsg> pending;

        while (true)
        {
            while (!_SendBufferIdle)
            {
                vTaskDelay(pdMS_TO_TICKS(SEND_THREAD_TICK_MS));
            }

            std::shared_ptr<LoraModule::LoraMessageInterface>* wrapper = nullptr;
            if (xQueueReceive(_sendQueue, &wrapper, 0) == pdTRUE)
            {
                auto msg = *wrapper;
                delete wrapper;

                bool isOwn = (msg->sender == LoraModule::Utilities::UserID());
                uint8_t attempts = isOwn
                                   ? std::max((uint8_t)1, LoraModule::Utilities::DefaultSendAttempts())
                                   : static_cast<uint8_t>(NUM_REBROADCAST_ATTEMPTS);

                pending.push_back({msg, attempts, static_cast<size_t>((rand() % MAX_SEND_DELAY_TICKS) + 1)});

                ESP_LOGI(TAG, "Queued msgID 0x%08X sender 0x%08X — %s — attempts %d",
                         msg->msgID, msg->sender,
                         isOwn ? "own" : "relay",
                         attempts);
            }

            for (auto it = pending.begin(); it != pending.end(); )
            {
                it->ticksLeft--;
                if (it->ticksLeft == 0)
                {
                    ESP_LOGI(TAG, "Transmitting msgID 0x%08X — attemptsLeft %d -> %d",
                             it->msg->msgID, it->attemptsLeft, it->attemptsLeft - 1);

                    size_t outLen = 0;
                    if (LoraModule::Utilities::SerializeMessage(it->msg, _SendBuffer, outLen))
                    {
                        _SendBufferLen  = outLen;
                        _SendBufferIdle = false;
                    }
                    else
                    {
                        ESP_LOGE(TAG, "Failed to serialize message");
                    }

                    it->attemptsLeft--;
                    if (it->attemptsLeft > 0)
                    {
                        it->ticksLeft = (rand() % MAX_SEND_DELAY_TICKS) + 1;
                        ++it;
                    }
                    else
                    {
                        ESP_LOGI(TAG, "msgID 0x%08X — all attempts exhausted, removing", it->msg->msgID);
                        it = pending.erase(it);
                    }

                    break;  // one message per tick
                }
                else
                {
                    ++it;
                }
            }

            vTaskDelay(pdMS_TO_TICKS(SEND_THREAD_TICK_MS));
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

    bool     _SendBufferIdle = true;
    uint8_t  _SendBuffer[MAX_MESSAGE_SIZE]{};
    size_t   _SendBufferLen = 0;
};

} // namespace LoraModule
