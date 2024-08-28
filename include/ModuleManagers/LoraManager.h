#pragma once

#include "LoraUtils.h"
#include "FilesystemUtils.h"
#include <RadioHead.h>
#include <RHGenericDriver.h>
#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include "Settings_Manager.h"

namespace 
{
    const size_t MAX_MESSAGE_SIZE = 255;
    const uint64_t BROADCAST_ID = 0x0;
    const uint8_t DEFAULT_NODE_ID = 1;
    const char *USER_LIST_FILENAME PROGMEM = "/SavedUsers.msgpk";
    const char *MESSAGE_LIST_FILENAME PROGMEM = "/SavedMessages.msgpk";
    const size_t MESSAGE_RECEIVE_TIMEOUT_MS = 10;
    const size_t RECEIVE_THREAD_SLEEP_MS = 1;
    const size_t NUM_REBROADCAST_ATTEMPTS = 1;
}

// Struct to manage message pointers waiting to send
struct QueuedMessageInfo
{
    MessageBase *msg;
    uint8_t numSendAttempts;
    size_t ticksLeft;
};

template <typename RadioType>
class LoraManager
{
public:
    // Constructor for manager with radios accepting a frequency and power level
    LoraManager(RadioType *driver, uint8_t cs, uint8_t intPin, uint8_t txPower)
    {
        _driver = driver;
        _cs = cs;
        _intPin = intPin;
        _txPower = txPower;
    }

    bool Init()
    {
        // Initialize the manager
        _manager = new RHReliableDatagram(*_driver, 1);
        _manager->setTimeout(10000);

        if (Settings_Manager::settings != nullptr)
        {
            uint32_t userID = Settings_Manager::settings["User"]["UserID"].as<uint32_t>();

#if DEBUG == 1
            Serial.print("UserID: 0x");

            Serial.println(userID, HEX);
#endif

            LoraUtils::SetUserID(userID);

            LoraUtils::SetNodeID(DEFAULT_NODE_ID);
            _freq = Settings_Manager::settings["Radio"]["Frequency"]["cfgVal"] | 915.0f;
            size_t cfgIdx = Settings_Manager::settings["Radio"]["Modem Config"]["cfgVal"].as<size_t>();

            // TODO: break different radio types into separate functions
            // if (typeid(RadioType) == typeid(RH_RF95))
            // {
            auto modemConfig = (RH_RF95::ModemConfigChoice)Settings_Manager::settings["Radio"]["Modem Config"]["vals"].as<JsonArray>()[cfgIdx].as<uint32_t>();
            _driver->setModemConfig(modemConfig);
            // }

            _transmitRetries = Settings_Manager::settings["Radio"]["Broadcast Retries"]["cfgVal"].as<uint8_t>();
        }
        else
        {
            _freq = 915.0f;
            _transmitRetries = 3;
        }

        _driver->setFrequency(_freq);
        _driver->setTxPower(_txPower);

        if(!_manager->init())
        {
            Serial.println("Radio Manager failed to initialize");
            return false;
        }

        LoraUtils::Init();

        LoraUtils::UserInfoListUpdated() += SaveUserInfoList;
        this->LoadUserInfoList(); 

        LoraUtils::SavedMessageListUpdated() += SaveMessageList;
        this->LoadMessageList();

        _sendQueue = System_Utils::getQueue(LoraUtils::MessageSendQueueID());

        if (_sendQueue == nullptr)
        {
            return false;
        }

        return true;
    }

    // RTOS task to handle receiving messages
    void ReceiveTask(void *pvParameters)
    {
        while (true) 
        {
            if (_manager->available())
            {
                uint8_t buffer[MAX_MESSAGE_SIZE];
                memset(buffer, 0, sizeof(buffer));

                uint8_t from;
                uint8_t to;
                uint8_t id;
                uint8_t flags;
                uint8_t len = sizeof(buffer);

                _manager->recvfromAckTimeout(buffer, &len, MESSAGE_RECEIVE_TIMEOUT_MS, &from, &to, &id, &flags);
                #if DEBUG == 1
                // Serial.println("Raw bytes:");
                // for (size_t i = 0; i < len; i++)
                // {
                //     Serial.print(buffer[i], HEX);
                //     Serial.print(" ");
                // }
                // Serial.println();
                {
                    StaticJsonDocument<MSG_BASE_SIZE> testDoc;
                    deserializeMsgPack(testDoc, (const char *)buffer, len);
                    Serial.println("Received message:");
                    serializeJson(testDoc, Serial);
                    Serial.println();
                }
                #endif
                auto msg = LoraUtils::DeserializeMessage(buffer, len);

                if (msg != nullptr)
                {   
                    #if DEBUG == 1
                    // Serial.print("Received message from 0x");
                    // Serial.print(msg->sender, HEX);
                    // Serial.println();
                    // Serial.printf("Length: %u\n", len);
                    // // auto jsondoc = msg->serializeJSON();
                    // StaticJsonDocument<MSG_BASE_SIZE> jsondoc;
                    // msg->serialize(jsondoc);
                    // serializeJson(jsondoc, Serial);
                    // Serial.println();
                    // Serial.println("End of message");
                    // Serial.println("Raw bytes:");
                    // for (size_t i = 0; i < len; i++)
                    // {
                    //     Serial.printf("%02X", buffer[i]);
                    //     Serial.print(" ");
                    // }
                    // Serial.println();
                    #endif

                    if (!msg->IsValid())
                    {
                        #if DEBUG == 1
                        Serial.println("Invalid message");
                        #endif
                        delete msg;
                        continue;
                    }

                    // Check if the message should be forwarded
                    auto fwd = ShouldMessageBeForwarded(msg);

                    // Dump message if it was sent from this node and came back
                    if (msg->sender == LoraUtils::UserID())
                    {
                        #if DEBUG == 1
                        // Serial.println("Message was sent from this node and came back");
                        #endif
                        delete msg;
                        continue;
                    }

                    if (fwd)
                    {
                        msg->bouncesLeft--;
                        LoraUtils::SendMessage(msg, NUM_REBROADCAST_ATTEMPTS); 

                        _lastReceivedMessages[msg->sender] = msg->msgID;
                    }
                    
                    // Check if the message is a broadcast or intended for this node
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
                }
            }

            vTaskDelay(pdMS_TO_TICKS(RECEIVE_THREAD_SLEEP_MS));
        }
    }

    // RTOS task for sending messages
    // This task will assign a randomized delay
    // to each message to prevent collisions
    void SendTask(void *pvParameters)
    {
        auto sendQueue = System_Utils::getQueue(LoraUtils::MessageSendQueueID());

        if (sendQueue == nullptr)
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

            if (xQueueReceive(sendQueue, &item, 0) == pdTRUE)
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
                    StaticJsonDocument<MSG_BASE_SIZE> doc;
                    auto success = msgTimer->msg->serialize(doc);                  

                    if (success)
                    {
                        #if DEBUG == 1
                        // Serial.println("Message serialized successfully. Message: ");
                        // serializeJson(doc, Serial);
                        // Serial.println();
                        #endif
                        len = measureMsgPack(doc) + 4;
                        uint8_t buffer[len];
                        len = serializeMsgPack(doc, buffer, len);

                        #if DEBUG == 1
                        Serial.println("Sending message:");
                        serializeJson(doc, Serial);
                        Serial.println();
                        // Serial.println("Sending raw bytes:");
                        // for (size_t i = 0; i < len; i++)
                        // {
                        //     Serial.printf("%02X", buffer[i]);
                        //     Serial.print(" ");
                        // }
                        // Serial.println();
                        #endif

                        _manager->sendtoWait(buffer, len, RH_BROADCAST_ADDRESS);

                        #if DEBUG == 1
                        // Serial.printf("Message sent of size: %u\n", len);
                        // Serial.println("Raw bytes:");
                        // for (size_t i = 0; i < len; i++)
                        // {
                        //     Serial.print(buffer[i], HEX);
                        //     Serial.print(" ");
                        // }
                        // Serial.println();
                        #endif
                    }
                    #if DEBUG == 1
                    else
                    {
                        Serial.println("Message failed to serialize");
                    }
                    #endif

                    msgTimer->numSendAttempts--;

                    if (msgTimer->msg->sender == LoraUtils::UserID())
                    {
                        // Update the last broadcast message.
                        LoraUtils::SetMyLastBroadcast(msgTimer->msg);
                    }

                    if (msgTimer->numSendAttempts > 0)
                    {
                        // Requeue the message
                        msgTimer->ticksLeft = (rand() % MAX_SEND_DELAY_TICKS) + 1;
                    }
                    else
                    {
                        // Delete the message
                        #if DEBUG == 1
                        // Serial.printf("Deleting message of ID: %u\n", msgTimer->msg->msgID);
                        #endif
                        delete msgTimer->msg;

                        // Remove from the vector
                        msgTimer = ticksUntilSend.erase(msgTimer);
                    }

                    // Only send one message per tick
                    break;
                }
            }

            vTaskDelay(SEND_THREAD_TICK_MS / portTICK_PERIOD_MS);      
        }
    }

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

    RHReliableDatagram *_manager;
    RadioType *_driver;
    uint8_t _cs;
    uint8_t _intPin;
    float _freq;
    uint8_t _txPower;
    uint8_t _transmitRetries;

    // Queue for messages to send
    QueueHandle_t _sendQueue;

    // map of last message received from each node to message ID
    // This is different from the received messages map in LoraUtils
    // The node will route messages not intended for it
    std::unordered_map<uint64_t, uint32_t> _lastReceivedMessages;

};
