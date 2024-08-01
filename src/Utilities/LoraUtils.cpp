#include "LoraUtils.h"

MessageBase *LoraUtils::_MyLastBroadcast = nullptr;
std::unordered_map<uint64_t, MessageBase *> LoraUtils::_ReceivedMessages;
EventHandlerT<uint64_t, bool> LoraUtils::_MessageReceived;
int LoraUtils::_MessageSendQueueID = -1;
uint64_t LoraUtils::_UserID = 0;
uint8_t LoraUtils::_NodeID = 0;
uint8_t LoraUtils::_DefaultSendAttempts = 3;

StaticSemaphore_t LoraUtils::_MessageAccessMutexBuffer;
SemaphoreHandle_t LoraUtils::_MessageAccessMutex;

StaticQueue_t LoraUtils::_MessageQueueBuffer;
uint8_t LoraUtils::_MessageQueueBufferStorage[MESSAGE_QUEUE_LENGTH * sizeof(OutboundMessageQueueItem)]; 

std::unordered_map<uint8_t, MessageDeserializer> LoraUtils::_deserializers;

void LoraUtils::Init() 
{
    _MessageAccessMutex = xSemaphoreCreateMutexStatic(&_MessageAccessMutexBuffer);
    _MessageSendQueueID =  System_Utils::registerQueue(MESSAGE_QUEUE_LENGTH, sizeof(OutboundMessageQueueItem), _MessageQueueBufferStorage, _MessageQueueBuffer);
}

bool LoraUtils::SendMessage(MessageBase *msg, uint8_t numSendAttempts) {
    if (msg == nullptr) {
        return false;
    }

    if (_MessageSendQueueID == -1) {
        return false;
    }

    if (numSendAttempts == 0) {
        numSendAttempts = _DefaultSendAttempts;
    }

    OutboundMessageQueueItem item = {msg, numSendAttempts};

    return System_Utils::sendToQueue(_MessageSendQueueID, &item, 1000);
}

// TODO: Return a copy of the message instead of the original pointer
MessageBase *LoraUtils::MyLastBroacast() {
    if (xSemaphoreTake(_MessageAccessMutex, portMAX_DELAY) == pdTRUE) {
        MessageBase *msg = _MyLastBroadcast->clone();
        xSemaphoreGive(_MessageAccessMutex);
        return msg;
    }
}

// TODO: Return a copy of the message instead of the original pointer
MessageBase *LoraUtils::ReceivedMessage(uint64_t userID) {
    if (xSemaphoreTake(_MessageAccessMutex, portMAX_DELAY) == pdTRUE) {
        MessageBase *msg = _ReceivedMessages[userID]->clone();
        xSemaphoreGive(_MessageAccessMutex);
        return msg;
    }
}

void LoraUtils::SetMyLastBroadcast(MessageBase *msg) {
    if (xSemaphoreTake(_MessageAccessMutex, portMAX_DELAY) == pdTRUE) {
        if (_MyLastBroadcast != nullptr && _MyLastBroadcast != msg) {
            delete _MyLastBroadcast;
        }
        _MyLastBroadcast = msg;
        xSemaphoreGive(_MessageAccessMutex);
    }
}

void LoraUtils::SetReceivedMessage(uint64_t userID, MessageBase *msg) {
    if (xSemaphoreTake(_MessageAccessMutex, portMAX_DELAY) == pdTRUE) {
        if (_ReceivedMessages.find(userID) != _ReceivedMessages.end()
            && _ReceivedMessages[userID] != msg) {
            delete _ReceivedMessages[userID];
        }
        _ReceivedMessages[userID] = msg;
        xSemaphoreGive(_MessageAccessMutex);
    }
}

bool LoraUtils::RegisterMessageDeserializer(uint8_t msgType, MessageDeserializer deserializer)
{
    if (_deserializers.find(msgType) != _deserializers.end())
    {
        return false;
    }

    _deserializers[msgType] = deserializer;
    return true;
}

MessageBase *LoraUtils::DeserializeMessage(uint8_t *buffer, size_t len)
{
    auto msgType = MessageBase::getMessageType(buffer);

    if (_deserializers.find(msgType) == _deserializers.end())
    {
        return nullptr;
    }

    return _deserializers[msgType](buffer, len);
}

bool LoraUtils::MessageExists(uint64_t userID, uint32_t msgID)
{
    if (xSemaphoreTake(_MessageAccessMutex, portMAX_DELAY) == pdTRUE) {
        if (_ReceivedMessages.find(userID) != _ReceivedMessages.end()) {
            if (_ReceivedMessages[userID]->msgID == msgID) {
                xSemaphoreGive(_MessageAccessMutex);
                return true;
            }
        }
        xSemaphoreGive(_MessageAccessMutex);
    }
    return false;
}