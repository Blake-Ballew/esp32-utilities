#pragma once

#include "System_Utils.h"
#include "MessageBase.h"
#include <ArduinoJson.h>
#include <map>
#include "EventHandler.h"

using MessageDeserializer = MessageBase *(*)(uint8_t *buffer, size_t len);

namespace
{
    const size_t MESSAGE_QUEUE_LENGTH = 8; // Number of messages that can be queued for sending
}

struct OutboundMessageQueueItem
{
    MessageBase *msg;
    uint8_t numSendAttempts;
};

class LoraUtils
{
public:
    static void Init();

    // Queues a message for the manager to send. This will create a copy of the message. The caller is responsible for deleting the original
    static bool SendMessage(MessageBase *msg, uint8_t numSendAttempts = 0);

    // Marks a message as opened
    static void MarkMessageOpened(uint64_t userID);

    // Getting and setting messages
    // Getting messages should return a pointer to a new copy that the caller is responsible for deleting
    static MessageBase *MyLastBroacast();
    static MessageBase *ReceivedMessage(uint64_t userID);

    static void SetMyLastBroadcast(MessageBase *msg);
    static void SetReceivedMessage(uint64_t userID, MessageBase *msg);

    // Register a deserializer for a message type
    static bool RegisterMessageDeserializer(uint8_t msgType, MessageDeserializer deserializer);

    // Deserialize a message from a buffer
    static MessageBase *DeserializeMessage(uint8_t *buffer, size_t len);
    static MessageBase *DeserializeMessage(JsonDocument &jsonDoc);

    // Check if a message exists in the received message map and is the same message ID
    static bool MessageExists(uint64_t userID, uint32_t msgID);

    // Serialize a JsonDocument to a MessagePack buffer, Deserialize it, and check for data loss
    static bool MessagePackSanityCheck(JsonDocument &jsonDoc);

    // Getters
    static int MessageSendQueueID() { return _MessageSendQueueID; }
    static uint32_t UserID() { return _UserID; }
    static uint8_t NodeID() { return _NodeID; }
    static EventHandlerT<uint32_t, bool> &MessageReceived() { return _MessageReceived; }
    static uint8_t DefaultSendAttempts() { return _DefaultSendAttempts; }
    static size_t GetNumMessages() { return _ReceivedMessages.size(); }
    static size_t GetNumUnreadMessages() 
    {
        size_t count = 0;
        xSemaphoreTake(_MessageAccessMutex, portMAX_DELAY);
        for (auto it = _ReceivedMessages.begin(); it != _ReceivedMessages.end(); it++)
        {
            if (!it->second->messageOpened)
                count++;
        }
        xSemaphoreGive(_MessageAccessMutex);
        return count;
    }

    // Setters
    static void SetMessageSendQueueID(int id) { _MessageSendQueueID = id; }
    static void SetUserID(uint32_t id) { _UserID = id; }
    static void SetNodeID(uint8_t id) { _NodeID = id; }
    static void SetDefaultSendAttempts(uint8_t num) { _DefaultSendAttempts = num; }

    // Managed iterators

    // All messages
    static void ResetMessageIterator() 
    {
        if (xSemaphoreTake(_MessageAccessMutex, portMAX_DELAY) == pdTRUE)
        {
            _ReceivedMessageIterator = _ReceivedMessages.begin();
            xSemaphoreGive(_MessageAccessMutex);
        }
    }

    static void IncrementMessageIterator() 
    { 
        if (xSemaphoreTake(_MessageAccessMutex, portMAX_DELAY) == pdTRUE)
        {
            if (_ReceivedMessages.size() > 0)
            {
                _ReceivedMessageIterator++;
            }

            if (_ReceivedMessageIterator == _ReceivedMessages.end())
            {
                _ReceivedMessageIterator = _ReceivedMessages.begin();
            }
            xSemaphoreGive(_MessageAccessMutex);
        }
    }

    static void DecrementMessageIterator()
    {
        if (xSemaphoreTake(_MessageAccessMutex, portMAX_DELAY) == pdTRUE)
        {
            if (_ReceivedMessages.size() > 0)
            {
                if (_ReceivedMessageIterator == _ReceivedMessages.begin())
                {
                    _ReceivedMessageIterator = _ReceivedMessages.end();
                }
                _ReceivedMessageIterator--;
            }
            xSemaphoreGive(_MessageAccessMutex);
        }
    }

    static MessageBase *GetCurrentMessage()
    {
        if (xSemaphoreTake(_MessageAccessMutex, portMAX_DELAY) == pdTRUE)
        {
            if (_ReceivedMessageIterator != _ReceivedMessages.end())
            {
                auto msg = _ReceivedMessageIterator->second->clone();
                xSemaphoreGive(_MessageAccessMutex);
                return msg;
            }
            xSemaphoreGive(_MessageAccessMutex);
        }
        return nullptr;
    }

    static bool IsMessageIteratorAtBeginning()
    {
        return _ReceivedMessageIterator == GetReceivedMessageBegin();
    }

    static bool IsMessageIteratorAtEnd()
    {
        return _ReceivedMessageIterator == GetReceivedMessageEnd();
    }

    // Unread messages
    static void ResetUnreadMessageIterator()
    {
        if (xSemaphoreTake(_MessageAccessMutex, portMAX_DELAY) == pdTRUE)
        {
            _UnreadMessageIterator = _ReceivedMessages.begin();
            while (_UnreadMessageIterator != _ReceivedMessages.end() && _UnreadMessageIterator->second->messageOpened)
            {
                _UnreadMessageIterator++;
            }
            xSemaphoreGive(_MessageAccessMutex);
        }
    }

    static void IncrementUnreadMessageIterator()
    {
        if (xSemaphoreTake(_MessageAccessMutex, portMAX_DELAY) == pdTRUE)
        {
            if (_ReceivedMessages.size() > 0)
            {
                _UnreadMessageIterator++;
                while (_UnreadMessageIterator != _ReceivedMessages.end() && _UnreadMessageIterator->second->messageOpened)
                {
                    _UnreadMessageIterator++;
                }
            }

            if (_UnreadMessageIterator == _ReceivedMessages.end())
            {
                _UnreadMessageIterator = _ReceivedMessages.begin();
                while (_UnreadMessageIterator != _ReceivedMessages.end() && _UnreadMessageIterator->second->messageOpened)
                {
                    _UnreadMessageIterator++;
                }
            }
            xSemaphoreGive(_MessageAccessMutex);
        }
    }

    static void DecrementUnreadMessageIterator()
    {
        if (xSemaphoreTake(_MessageAccessMutex, portMAX_DELAY) == pdTRUE)
        {
            if (_ReceivedMessages.size() > 0)
            {
                if (_UnreadMessageIterator == _ReceivedMessages.begin())
                {
                    _UnreadMessageIterator = _ReceivedMessages.end();
                }
                _UnreadMessageIterator--;
                while (_UnreadMessageIterator != _ReceivedMessages.begin() && _UnreadMessageIterator->second->messageOpened)
                {
                    _UnreadMessageIterator--;
                }
            }
            xSemaphoreGive(_MessageAccessMutex);
        }
    }

    static MessageBase *GetCurrentUnreadMessage()
    {
        if (xSemaphoreTake(_MessageAccessMutex, portMAX_DELAY) == pdTRUE)
        {
            if (_UnreadMessageIterator != _ReceivedMessages.end())
            {
                auto msg = _UnreadMessageIterator->second->clone();
                xSemaphoreGive(_MessageAccessMutex);
                return msg;
            }
            xSemaphoreGive(_MessageAccessMutex);
        }
        return nullptr;
    }

    static bool IsUnreadMessageIteratorAtBeginning()
    {
        return _UnreadMessageIterator == GetUnreadMessageBegin();
    }

    static bool IsUnreadMessageIteratorAtEnd()
    {
        return _UnreadMessageIterator == GetUnreadMessageEnd();
    }

protected:
    // Last message received from each user
    static std::map<uint32_t, MessageBase *> _ReceivedMessages;

    // Last message broadcasted by this device
    static MessageBase *_MyLastBroadcast;

    // Invoked when a message is received with the UserID of the sender and
    // a boolean indicating if the message is new or an update of an old message
    static EventHandlerT<uint32_t, bool> _MessageReceived;

    // ID for queue to send outgoing messages to manager class
    static int _MessageSendQueueID;

    // UserID of this device
    static uint32_t _UserID;
    
    // NodeID of this device
    static uint8_t _NodeID;

    // Default number of send attempts for a message
    static uint8_t _DefaultSendAttempts;

    // Semaphore for accessing _MyLastBroadcast and _ReceivedMessages
    static SemaphoreHandle_t _MessageAccessMutex;
    static StaticSemaphore_t _MessageAccessMutexBuffer;

    // Static message queue data
    static StaticQueue_t _MessageQueueBuffer;
    static uint8_t _MessageQueueBufferStorage[MESSAGE_QUEUE_LENGTH * sizeof(OutboundMessageQueueItem)];

    // Mapping message types to deserializers
    static std::unordered_map<uint8_t, MessageDeserializer> _deserializers;

    // Iterators
    static std::map<uint32_t, MessageBase *>::iterator _ReceivedMessageIterator;
    static std::map<uint32_t, MessageBase *>::iterator _UnreadMessageIterator;

    static std::map<uint32_t, MessageBase *>::iterator GetReceivedMessageBegin()
    {
        return _ReceivedMessages.begin();
    }

    static std::map<uint32_t, MessageBase *>::iterator GetReceivedMessageEnd()
    {
        return _ReceivedMessages.end();
    }

    static std::map<uint32_t, MessageBase *>::iterator GetUnreadMessageBegin()
    {
        auto it = _ReceivedMessages.begin();
        if (xSemaphoreTake(_MessageAccessMutex, portMAX_DELAY) == pdTRUE)
        {
            while (it != _ReceivedMessages.end() && it->second->messageOpened)
            {
                it++;
            }
            xSemaphoreGive(_MessageAccessMutex);
        }
        return it;
    }

    static std::map<uint32_t, MessageBase *>::iterator GetUnreadMessageEnd()
    {
        return _ReceivedMessages.end();
    }
};