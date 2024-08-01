#pragma once

#include "System_Utils.h"
#include "MessageBase.h"
#include <ArduinoJson.h>
#include <unordered_map>
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

    // Queues a message for the manager to send. The manager is responsible for managing the memory of the message
    static bool SendMessage(MessageBase *msg, uint8_t numSendAttempts);

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

    // Check if a message exists in the received message map and is the same message ID
    static bool MessageExists(uint64_t userID, uint32_t msgID);

    // Getters
    static int MessageSendQueueID() { return _MessageSendQueueID; }
    static uint64_t UserID() { return _UserID; }
    static uint8_t NodeID() { return _NodeID; }
    static EventHandlerT<uint64_t, bool> &MessageReceived() { return _MessageReceived; }
    static uint8_t DefaultSendAttempts() { return _DefaultSendAttempts; }

    // Setters
    static void SetMessageSendQueueID(int id) { _MessageSendQueueID = id; }
    static void SetUserID(uint64_t id) { _UserID = id; }
    static void SetNodeID(uint8_t id) { _NodeID = id; }
    static void SetDefaultSendAttempts(uint8_t num) { _DefaultSendAttempts = num; }

protected:
    // Last message received from each user
    static std::unordered_map<uint64_t, MessageBase *> _ReceivedMessages;

    // Last message broadcasted by this device
    static MessageBase *_MyLastBroadcast;

    // Invoked when a message is received with the UserID of the sender and
    // a boolean indicating if the message is new or an update of an old message
    static EventHandlerT<uint64_t, bool> _MessageReceived;

    // ID for queue to send outgoing messages to manager class
    static int _MessageSendQueueID;

    // UserID of this device
    static uint64_t _UserID;
    
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
};