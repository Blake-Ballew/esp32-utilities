#pragma once

#include "LoraUtils.h"
#include "FilesystemUtils.h"
#include "LoraDriverInterface.h"
#include "Settings_Manager.h"

namespace 
{
    const size_t MAX_MESSAGE_SIZE = 512;
    const uint64_t BROADCAST_ID = 0x0;
    const uint8_t DEFAULT_NODE_ID = 1;
    const char *USER_LIST_FILENAME PROGMEM = "/SavedUsers.msgpk";
    const char *MESSAGE_LIST_FILENAME PROGMEM = "/SavedMessages.msgpk";

    const size_t MESSAGE_RECEIVE_TIMEOUT_MS = 100;
    const size_t RECEIVE_THREAD_SLEEP_MS = 100;

    const size_t AFTER_SEND_BLOCK_TIME_MS = 500;

    const size_t NUM_REBROADCAST_ATTEMPTS = 1;

    const size_t SEND_THREAD_MUTEX_ADDITIONAL_TIME_MS = 200;
}

// Struct to manage message pointers waiting to send
struct QueuedMessageInfo
{
    MessageBase *msg;
    uint8_t numSendAttempts;
    size_t ticksLeft;
};

enum RpcChannelState
{
    RECEIVING = 0,
    SENDING = 1
};

// template <typename RadioType>
class LoraManager
{
public:
    // Constructor for manager with radios accepting a frequency and power level
    LoraManager(LoraDriverInterface *driver) : _Driver(driver)
    {

    }

    bool Init()
    {
        // Initialize the manager
        // _manager = new RHReliableDatagram(*_driver, 1);
        // _manager->setTimeout(10000);

        // if(!_manager->init())
        // {
        //     Serial.println("Radio Manager failed to initialize");
        //     return false;
        // }

        // // Initialize mutex
        // _RadioMutex = xSemaphoreCreateMutex();

        if (_Driver == nullptr)
        {
            return false;
        }

        if (!_Driver->Init())
        {
            return false;
        }

        LoraUtils::Init();

        LoraUtils::UserInfoListUpdated() += SaveUserInfoList;
        this->LoadUserInfoList(); 

        LoraUtils::SavedMessageListUpdated() += SaveMessageList;
        this->LoadMessageList();

        if (LoraUtils::GetSavedMessageListSize() == 0)
        {
            LoraUtils::AddSavedMessage("Meet here");
            LoraUtils::AddSavedMessage("Point of interest");
            LoraUtils::AddSavedMessage("I have a quest");
        }

        _sendQueue = System_Utils::getQueue(LoraUtils::MessageSendQueueID());

        if (_sendQueue == nullptr)
        {
            return false;
        }

        return true;
    }

    void RadioTask()
    {
        while (true)
        {
            StaticJsonDocument<MSG_BASE_SIZE> jsondoc;

            if (_Driver->ReceiveMessage(jsondoc, MESSAGE_RECEIVE_TIMEOUT_MS))
            {
                #if DEBUG == 1
                Serial.print("Message received: ");
                serializeJson(jsondoc, Serial);
                Serial.println();
                #endif
                // Process received message
                if (MessageBase::GetMessageTypeFromJson(jsondoc) != 0)
                {
                    MessageBase *msg = LoraUtils::DeserializeMessage(jsondoc);

                    if (msg != nullptr)
                    {
                        if (!msg->IsValid())
                        {
#if DEBUG == 1
                            Serial.println("Invalid message");
#endif
                            delete msg;   
                            msg = nullptr;
                        }
                        else
                        {
                            // Fill in time received if not set
                            if (msg->time == 0 && msg->date == 0 && NavigationUtils::IsGPSConnected())
                            {
                                msg->time = NavigationUtils::GetTime().value();
                                msg->date = NavigationUtils::GetDate().value();
                            }

                            auto fwd = ShouldMessageBeForwarded(msg);

                            // Dump message if it was sent from this node and came back
                            if (msg->sender == LoraUtils::UserID())
                            {
                                #if DEBUG == 1
                                // Serial.println("Message was sent from this node and came back");
                                #endif

                                delete msg;
                                msg = nullptr;
                            }
                            else
                            {
                                if (fwd)
                                {
                                    msg->bouncesLeft--;
                                    LoraUtils::SendMessage(msg, NUM_REBROADCAST_ATTEMPTS); 

                                    _lastReceivedMessages[msg->sender] = msg->msgID;
                                }

                                if (msg->recipient == LoraUtils::UserID() || msg->recipient == BROADCAST_ID)
                                {
                                    #if DEBUG == 1
                                    // Serial.println("Message directed to this node or broadcast");
                                    #endif
                                    // Handle the message
                                    auto msgExists = LoraUtils::MessageExists(msg->sender, msg->msgID);
                                    LoraUtils::SetReceivedMessage(msg->sender, msg);

                                    LoraUtils::MessageReceived().Invoke(msg->sender, !msgExists);
                                }

                                delete msg;
                                msg = nullptr;
                            }
                        }
                    }
                }
            }

            if (!_SendBufferIdle)
            {
                if (MessageBase::GetMessageTypeFromJson(_SendBuffer) != 0)
                {
                    if (!_Driver->SendMessage(_SendBuffer))
                    {
                        #if DEBUG == 1
                        Serial.println("Failed to send message");
                        #endif
                    }
                    _SendBuffer.clear();
                    _SendBufferIdle = true;
                    vTaskDelay(pdMS_TO_TICKS(AFTER_SEND_BLOCK_TIME_MS));
                    continue;
                }
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

        const size_t SEND_THREAD_TICK_MS = 250;
        const size_t MAX_SEND_DELAY_TICKS = 15;

        // Map of messages to ticks until send
        std::vector<QueuedMessageInfo> ticksUntilSend;

        while (true)
        {
            OutboundMessageQueueItem item;

            while (!_SendBufferIdle)
            {
                vTaskDelay(pdMS_TO_TICKS(SEND_THREAD_TICK_MS));
            }

            if (xQueueReceive(_sendQueue, &item, 0) == pdTRUE)
            {
                QueuedMessageInfo info = {item.msg, item.numSendAttempts, 0};

                #if DEBUG == 1
                // Serial.print("Message queued to send: ");
                // StaticJsonDocument<MSG_BASE_SIZE> jsondoc;
                // item.msg->serialize(jsondoc);
                // serializeJson(jsondoc, Serial);
                // Serial.println();
                // Serial.println("Sanity checking message:");
                // LoraUtils::MessagePackSanityCheck(jsondoc);
                // Serial.print("Message length: ");
                // Serial.println(measureMsgPack(jsondoc));
                #endif

                // Assign a random delay to the message
                info.ticksLeft = (rand() % MAX_SEND_DELAY_TICKS) + 1;

                ticksUntilSend.push_back(info);
            }

            for (auto msgTimer = ticksUntilSend.begin(); msgTimer != ticksUntilSend.end(); msgTimer++)
            {
                msgTimer->ticksLeft--;

                if (msgTimer->ticksLeft == 0)
                {
                    size_t len = 0;

                    // Send the message
                    _SendBuffer.clear();
                    auto success = msgTimer->msg->serialize(_SendBuffer);                  

                    if (success)
                    {
                        _SendBufferIdle = false;
                    }
                    #if DEBUG == 1
                    else
                    {
                        Serial.println("Message failed to serialize");
                    }
                    #endif

                    msgTimer->numSendAttempts--;

                    // if (msgTimer->msg->sender == LoraUtils::UserID())
                    // {
                    //     // Update the last broadcast message.
                    //     LoraUtils::SetMyLastBroadcast(msgTimer->msg);
                    // }

                    if (msgTimer->numSendAttempts > 0)
                    {
                        // Requeue the message
                        msgTimer->ticksLeft = (rand() % MAX_SEND_DELAY_TICKS) + 1;
                    }
                    else
                    {
                        // Delete the message
                        delete msgTimer->msg;

                        // Remove from the vector
                        msgTimer = ticksUntilSend.erase(msgTimer);
                    }

                    // Only send one message per tick
                    break;
                }
            }

            vTaskDelay(pdMS_TO_TICKS(SEND_THREAD_TICK_MS));
        }
    }

    // Read RPC requests over LoRa
    // bool ReadRpcRequests(uint32_t channelID, JsonDocument &payload)
    // {
    //     // Skip reading for messages if we're still sending a reply
    //     if (xTaskGetTickCount() < _NextReceiveTime)
    //     {
    //         return false;
    //     }

    //     if (_Driver->ReceiveMessage(payload, MESSAGE_RECEIVE_TIMEOUT_MS))
    //     {
    //         return true;
    //     }

    //     return false;
    // }

    // void QueueRpcResponse(uint32_t channelID, JsonDocument &payload)
    // {

    // }

    // Event code to save user list to flash
    static void SaveUserInfoList()
    {
        DynamicJsonDocument doc(1024);
        LoraUtils::SerializeUserInfoList(doc);

        auto returncode = FilesystemUtils::WriteFile(USER_LIST_FILENAME, doc);

        if (returncode != FilesystemReturnCode::FILESYSTEM_OK)
        {
            #if DEBUG == 1
            Serial.print("Failed to save user list. Error code: ");
            Serial.println((int)returncode);
            #endif
        }
    }

    // Load user list from flash
    void LoadUserInfoList()
    {
        DynamicJsonDocument doc(1024);
        auto returncode = FilesystemUtils::ReadFile(USER_LIST_FILENAME, doc);

        if (returncode != FilesystemReturnCode::FILESYSTEM_OK)
        {
            #if DEBUG == 1
            Serial.print("Failed to load user list. Error code: ");
            Serial.println((int)returncode);
            #endif
        }
        else
        {
            LoraUtils::DeserializeUserInfoList(doc);
        }
    }

    // Event code to save message list to flash
    static void SaveMessageList()
    {
        DynamicJsonDocument doc(1024);
        LoraUtils::SerializeSavedMessageList(doc);

        auto returncode = FilesystemUtils::WriteFile(MESSAGE_LIST_FILENAME, doc);

        if (returncode != FilesystemReturnCode::FILESYSTEM_OK)
        {
            #if DEBUG == 1
            Serial.print("Failed to save message list. Error code: ");
            Serial.println((int)returncode);
            #endif
        }
    }

    // Load message list from flash
    void LoadMessageList()
    {
        DynamicJsonDocument doc(1024);
        auto returncode = FilesystemUtils::ReadFile(MESSAGE_LIST_FILENAME, doc);

        if (returncode != FilesystemReturnCode::FILESYSTEM_OK)
        {
            #if DEBUG == 1
            Serial.print("Failed to load message list. Error code: ");
            Serial.println((int)returncode);
            #endif
        }
        else
        {
            LoraUtils::DeserializeSavedMessageList(doc);
        }
    }

    void SetTaskHandles(TaskHandle_t sendHandle, TaskHandle_t receiveHandle)
    {
        _SendTaskHandle = sendHandle;
        _ReceiveTaskHandle = receiveHandle;
    }

protected:

    bool ShouldMessageBeForwarded(MessageBase *msg) 
    {
        if (msg == nullptr)
        {
            #if DEBUG == 1
            // Serial.println("Message is null");
            #endif
            return false;
        }

        auto senderID = msg->sender;
        auto msgID = msg->msgID;

        // Don't forward messages that came from this node
        if (senderID == LoraUtils::UserID())
        {
            #if DEBUG == 1
            // Serial.println("Message came from this node");
            #endif
            return false;
        }

        if (msg->bouncesLeft == 0)
        {
            #if DEBUG == 1
            // Serial.println("Message has no bounces left");
            #endif
            return false;
        }

        // Check if the message has been received before
        if (_lastReceivedMessages.find(senderID) != _lastReceivedMessages.end())
        {
            if (_lastReceivedMessages[senderID] == msgID)
            {
                #if DEBUG == 1
                // Serial.println("Message has already been received");
                #endif
                return false;
            }
            else
            {
                #if DEBUG == 1
                // Serial.println("Message has not been received. Resending");
                #endif
                return true;
            }
        }
        else
        {
            #if DEBUG == 1
            // Serial.println("Message from new user. Resending");
            #endif
            return true;
        }
    }

    // RHReliableDatagram *_manager;
    // RHGenericDriver *_driver;
    // uint8_t _cs;
    // uint8_t _intPin;
    // float _freq;
    // uint8_t _txPower;

    // Lora Driver
    LoraDriverInterface *_Driver;

    // Queue for messages to send
    QueueHandle_t _sendQueue;

    // map of last message received from each node to message ID
    // This is different from the received messages map in LoraUtils
    // The node will route messages not intended for it
    std::unordered_map<uint64_t, uint32_t> _lastReceivedMessages;

    // Task handles
    TaskHandle_t _SendTaskHandle = nullptr;
    TaskHandle_t _ReceiveTaskHandle = nullptr;

    // Mutex for use of the LoRa Radio
    SemaphoreHandle_t _RadioMutex = nullptr;

    // Send buffer
    TickType_t _NextReceiveTime = 0;
    bool _SendBufferIdle = true;
    DynamicJsonDocument _SendBuffer = DynamicJsonDocument(MAX_MESSAGE_SIZE);
    
};
