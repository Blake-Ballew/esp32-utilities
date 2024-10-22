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

struct UserInfo
{
    uint32_t UserID;
    std::string Name;
};

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
    static std::string UserName() { return _UserName; }
    static uint8_t NodeID() { return _NodeID; }
    static EventHandlerT<uint32_t, bool> &MessageReceived() { return _MessageReceived; }
    static uint8_t DefaultSendAttempts() { return _DefaultSendAttempts; }
    static size_t GetNumMessages() { return _ReceivedMessages.size(); }
    static size_t GetNumUnreadMessages() { return _UnreadMessages.size(); }
    static bool MyLastBroacastExists() { return _MyLastBroadcast != nullptr; }

    // Setters
    static void SetMessageSendQueueID(int id) { _MessageSendQueueID = id; }
    static void SetUserID(uint32_t id) { _UserID = id; }
    static void SetUserName(std::string name) { _UserName = name; } 
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
            if (_ReceivedMessageIterator != _ReceivedMessages.end())
            {
                _ReceivedMessageIterator++;
            }

            xSemaphoreGive(_MessageAccessMutex);
        }
    }

    static void DecrementMessageIterator()
    {
        if (xSemaphoreTake(_MessageAccessMutex, portMAX_DELAY) == pdTRUE)
        {
            if (_ReceivedMessageIterator != _ReceivedMessages.begin())
            {
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

    static uint32_t GetCurrentMessageSenderID()
    {
        if (xSemaphoreTake(_MessageAccessMutex, portMAX_DELAY) == pdTRUE)
        {
            if (_ReceivedMessageIterator != _ReceivedMessages.end())
            {
                xSemaphoreGive(_MessageAccessMutex);
                return _ReceivedMessageIterator->first;
            }
            xSemaphoreGive(_MessageAccessMutex);
        }
        return 0;
    }

    static bool IsMessageIteratorAtBeginning()
    {
        return _ReceivedMessageIterator == _ReceivedMessages.begin();
    }

    static bool IsMessageIteratorAtEnd()
    {
        return _ReceivedMessageIterator == _ReceivedMessages.end();
    }

    // Unread messages
    static void ResetUnreadMessageIterator()
    {
        if (xSemaphoreTake(_MessageAccessMutex, portMAX_DELAY) == pdTRUE)
        {
            _UnreadMessageIterator = _UnreadMessages.begin();
            xSemaphoreGive(_MessageAccessMutex);
        }
    }

    static void IncrementUnreadMessageIterator()
    {
        if (xSemaphoreTake(_MessageAccessMutex, portMAX_DELAY) == pdTRUE)
        {
            if (_UnreadMessageIterator != _UnreadMessages.end())
            {
                _UnreadMessageIterator++;
            }

            xSemaphoreGive(_MessageAccessMutex);
        }
    }

    static void DecrementUnreadMessageIterator()
    {
        if (xSemaphoreTake(_MessageAccessMutex, portMAX_DELAY) == pdTRUE)
        {
            if (_UnreadMessageIterator != _UnreadMessages.begin())
            {
                _UnreadMessageIterator--;
            }

            xSemaphoreGive(_MessageAccessMutex);
        }
    }

    static MessageBase *GetCurrentUnreadMessage()
    {
        if (xSemaphoreTake(_MessageAccessMutex, portMAX_DELAY) == pdTRUE)
        {
            if (_UnreadMessageIterator != _UnreadMessages.end())
            {
                auto msg = _UnreadMessageIterator->second->clone();
                xSemaphoreGive(_MessageAccessMutex);
                return msg;
            }
            xSemaphoreGive(_MessageAccessMutex);
        }
        return nullptr;
    }

    static uint32_t GetCurrentUnreadMessageSenderID()
    {
        if (xSemaphoreTake(_MessageAccessMutex, portMAX_DELAY) == pdTRUE)
        {
            if (_UnreadMessageIterator != _UnreadMessages.end())
            {
                xSemaphoreGive(_MessageAccessMutex);
                return _UnreadMessageIterator->first;
            }
            xSemaphoreGive(_MessageAccessMutex);
        }
        return 0;
    }

    static bool IsUnreadMessageIteratorAtBeginning()
    {
        return _UnreadMessageIterator == _UnreadMessages.begin();
    }

    static bool IsUnreadMessageIteratorAtEnd()
    {
        return _UnreadMessageIterator == _UnreadMessages.end();
    }

    static EventHandler &SavedMessageListUpdated() { return _SavedMessageListUpdated; }

    static std::vector<std::string>::iterator SavedMessageListBegin() { return _SavedMessageList.begin(); }
    static std::vector<std::string>::iterator SavedMessageListEnd() { return _SavedMessageList.end(); }
    static void AddSavedMessage(std::string message, bool flash = true);
    static void DeleteSavedMessage(std::vector<std::string>::iterator &it);
    static void ClearSavedMessages();
    static void UpdateSavedMessage(std::vector<std::string>::iterator &it, std::string message);
    static size_t GetSavedMessageListSize() { return _SavedMessageList.size(); }

    static void SerializeSavedMessageList(JsonDocument &doc);
    static void DeserializeSavedMessageList(JsonDocument &doc);

    static EventHandler &UserInfoListUpdated() { return _UserInfoListUpdated; }

    static std::vector<UserInfo>::iterator UserInfoListBegin() { return _UserInfoList.begin(); }
    static std::vector<UserInfo>::iterator UserInfoListEnd() { return _UserInfoList.end(); }
    static void AddUserInfo(UserInfo userInfo);
    static void DeleteUserInfo(std::vector<UserInfo>::iterator &it);
    static void UpdateUserInfo(std::vector<UserInfo>::iterator &it, UserInfo userInfo);

    static void SerializeUserInfoList(JsonDocument &doc);
    static void DeserializeUserInfoList(JsonDocument &doc);

    static void FlashDefaultMessages();

protected:
    // Last message received from each user
    static std::map<uint32_t, MessageBase *> _ReceivedMessages;

    // Unread messages
    static std::map<uint32_t, MessageBase *> _UnreadMessages;

    // Last message broadcasted by this device
    static MessageBase *_MyLastBroadcast;

    // Invoked when a message is received with the UserID of the sender and
    // a boolean indicating if the message is new or an update of an old message
    static EventHandlerT<uint32_t, bool> _MessageReceived;

    // Invoked when the UserInfo list is updated
    static EventHandler _UserInfoListUpdated;

    // Invoked when saved message list is updated
    static EventHandler _SavedMessageListUpdated;

    // List of saved users
    static std::vector<UserInfo> _UserInfoList;

    // List of saved messages
    static std::vector<std::string> _SavedMessageList;

    // ID for queue to send outgoing messages to manager class
    static int _MessageSendQueueID;

    // UserID of this device
    static uint32_t _UserID;

    // User Name
    static std::string _UserName;
    
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

    
};