#pragma once

#include "LoraUtils.h"
#include <RadioHead.h>
#include <RHGenericDriver.h>
#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include "Settings_Manager.h"

namespace 
{
    const size_t MAX_MESSAGE_SIZE = 255;
    const uint64_t BROADCAST_ID = 0x0;
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
    LoraManager(RadioType &driver, uint8_t cs, uint8_t intPin, uint8_t txPower)
    {
        _driver = driver;
        _cs = cs;
        _intPin = intPin;
        _txPower = txPower;
    }

    bool Init()
    {
        // Initialize the manager
        _manager = RHReliableDatagram(_driver, 1);

        if (Settings_Manager::settings != nullptr)
        {
            JsonArray user = Settings_Manager::settings["User"]["UserID"].as<JsonArray>();
            uint64_t userID = 0;
            for (uint8_t i = 0; i < USERID_SIZE_BYTES; i++)
            {
                userID |= (uint64_t)user[i] << (((USERID_SIZE_BYTES - 1) - i) * 8);
            }

#if DEBUG == 1
            Serial.print("UserID: 0x");

            Serial.println(userID, HEX);
#endif

            LoraUtils::SetUserID(userID);

            LoraUtils::SetNodeID(DEFAULT_NODE_ID);
            _freq = Settings_Manager::settings["Radio"]["Frequency"]["cfgVal"] | 915.0f;
            size_t cfgIdx = Settings_Manager::settings["Radio"]["Modem Config"]["cfgVal"].as<size_t>();

            // TODO: break different radio types into separate functions
            if (typeid(RadioType) == typeid(RH_RF95))
            {
                auto modemConfig = (RH_RF95::ModemConfigChoice)Settings_Manager::settings["Radio"]["Modem Config"]["vals"].as<JsonArray>()[cfgIdx].as<uint32_t>();
                driver.setModemConfig(modemConfig);
            }

            _transmitRetries = Settings_Manager::settings["Radio"]["Broadcast Retries"]["cfgVal"].as<uint8_t>();
        }
        else
        {
            _freq = 915.0f;
            _transmitRetries = 3;
        }

        driver.setFrequency(_freq);
        // driver.setTxPower(_txPower);

        if(!_manager.init())
        {
            Serial.println("Radio Manager failed to initialize");
            return false;
        }

        LoraUtils::Init();
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
            uint8_t buffer[MAX_MESSAGE_SIZE];

            uint8_t from;
            uint8_t to;
            uint8_t id;
            uint8_t flags;
            uint8_t len = sizeof(buffer);

            memset(buffer, 0, sizeof(buffer));

            if (manager->recvfromAck(buffer, &len, &from, &to, &id, &flags))
            {
                auto msg = LoraUtils::DeserializeMessage(buffer, len);

                if (msg != nullptr)
                {   
                    // Check if the message should be forwarded
                    auto fwd = ShouldMessageBeForwarded(msg);

                    if (fwd)
                    {
                        // Forward the message
                        if (msg->bouncesLeft > 0)
                        {
                            msg->bouncesLeft--;
                            LoraUtils::SendMessage(msg, 1);

                            _lastReceivedMessages[msg->sender] = msg->msgID;
                        }
                    }
                    
                    // Check if the message is a broadcast or intended for this node
                    if (msg->recipient == LoraUtils::NodeID() || msg->recipient == BROADCAST_ID)
                    {
                        // Handle the message
                        auto msgExists = LoraUtils::MessageExists(msg->sender, msg->msgID);
                        LoraUtils::SetReceivedMessage(msg->sender, msg);

                        LoraUtils::MessageReceived().Invoke(msg->sender, msgExists);
                    }
                }
            }
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

        const size_t SEND_THREAD_TICK_MS = 100;
        const size_t MAX_SEND_DELAY_TICKS = 15;

        // Map of messages to ticks until send
        std::vector<QueuedMessageInfo> ticksUntilSend;

        while (true)
        {
            OutboundMessageQueueItem item;

            if (xQueueReceive(sendQueue, &item, 0) == pdTRUE)
            {
                QueuedMessageInfo info = {item.msg, item.numSendAttempts, 0};

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
                    auto buffer = msgTimer->msg->serialize(len);

                    if (buffer != nullptr)
                    {
                        _manager.sendtoWait(buffer, len, RH_BROADCAST_ADDRESS);

                        delete[] buffer;
                    }

                    msgTimer->numSendAttempts--;

                    if (msgTimer->numSendAttempts > 0)
                    {
                        // Requeue the message
                        msgTimer->ticksLeft = (rand() % MAX_SEND_DELAY_TICKS) + 1;
                    }
                    else if (msgTimer->msg->sender == LoraUtils::UserID())
                    {
                        // Update the last broadcast message. It will be deleted when overwritten
                        LoraUtils::SetMyLastBroadcast(msgTimer->msg);

                        // Remove from the vector
                        msgTimer = ticksUntilSend.erase(msgTimer);
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

            vTaskDelay(SEND_THREAD_TICK_MS / portTICK_PERIOD_MS);      
        }
    }

protected:

    bool ShouldMessageBeForwarded(MessageBase *msg) 
    {
        if (msg == nullptr)
        {
            return false;
        }

        auto senderID = msg->sender;
        auto msgID = msg->msgID;

        // Don't forward messages that came from this node
        if (senderID == LoraUtils::UserID())
        {
            return false;
        }

        // Check if the message has been received before
        if (_lastReceivedMessages.find(senderID) != _lastReceivedMessages.end())
        {
            if (_lastReceivedMessages[senderID] == msgID)
            {
                if (msg->bouncesLeft == 0)
                {
                    return false;
                }
                else
                {
                    return true;
                }
            }
        }
        else
        {
            return true;
        }
    }

    RHReliableDatagram _manager;
    RadioType &_driver;
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
