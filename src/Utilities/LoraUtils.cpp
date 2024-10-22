#include "LoraUtils.h"

std::map<uint32_t, MessageBase *> LoraUtils::_ReceivedMessages;
std::map<uint32_t, MessageBase *> LoraUtils::_UnreadMessages;

MessageBase *LoraUtils::_MyLastBroadcast = nullptr;

EventHandlerT<uint32_t, bool> LoraUtils::_MessageReceived;

EventHandler LoraUtils::_SavedMessageListUpdated;
std::vector<std::string> LoraUtils::_SavedMessageList;

EventHandler LoraUtils::_UserInfoListUpdated;
std::vector<UserInfo> LoraUtils::_UserInfoList;

int LoraUtils::_MessageSendQueueID = -1;

uint8_t LoraUtils::_DefaultSendAttempts = 3;

uint32_t LoraUtils::_UserID = 0;
std::string LoraUtils::_UserName = "User";
uint8_t LoraUtils::_NodeID = 0;

StaticSemaphore_t LoraUtils::_MessageAccessMutexBuffer;
SemaphoreHandle_t LoraUtils::_MessageAccessMutex;

StaticQueue_t LoraUtils::_MessageQueueBuffer;
uint8_t LoraUtils::_MessageQueueBufferStorage[MESSAGE_QUEUE_LENGTH * sizeof(OutboundMessageQueueItem)]; 

std::unordered_map<uint8_t, MessageDeserializer> LoraUtils::_deserializers;

std::map<uint32_t, MessageBase *>::iterator LoraUtils::_ReceivedMessageIterator;
std::map<uint32_t, MessageBase *>::iterator LoraUtils::_UnreadMessageIterator;

void LoraUtils::Init() 
{
    _MessageAccessMutex = xSemaphoreCreateMutexStatic(&_MessageAccessMutexBuffer);
    _MessageSendQueueID =  System_Utils::registerQueue(MESSAGE_QUEUE_LENGTH, sizeof(OutboundMessageQueueItem), _MessageQueueBufferStorage, _MessageQueueBuffer);
}

bool LoraUtils::SendMessage(MessageBase *msg, uint8_t numSendAttempts) {
    if (msg == nullptr) 
    {
        #if DEBUG == 1
        Serial.println("LoraUtils::SendMessage: msg is null");
        #endif
        return false;
    }

    if (_MessageSendQueueID == -1) 
    {
        #if DEBUG == 1
        Serial.println("LoraUtils::SendMessage: Message queue not initialized");
        #endif
        return false;
    }

    if (numSendAttempts == 0) 
    {
        numSendAttempts = _DefaultSendAttempts;
    }

    auto msgToSend = msg->clone();

    if (msgToSend->sender == _UserID)
    {
        SetMyLastBroadcast(msgToSend);
    }

    OutboundMessageQueueItem item = {msgToSend, numSendAttempts};

    #if DEBUG == 1
    Serial.print("LoraUtils::SendMessage: Sending message to queue. Type: ");
    Serial.println(msgToSend->GetInstanceMessageType());
    #endif
    return System_Utils::sendToQueue(_MessageSendQueueID, &item, 1000);
}

void LoraUtils::MarkMessageOpened(uint64_t userID) {
    if (xSemaphoreTake(_MessageAccessMutex, portMAX_DELAY) == pdTRUE) {
        if (_UnreadMessages.find(userID) != _UnreadMessages.end()) {
            delete _UnreadMessages[userID];

            if (_UnreadMessageIterator->first == userID) {
                _UnreadMessageIterator = _UnreadMessages.erase(_UnreadMessageIterator);
            } else {
                _UnreadMessages.erase(userID);
            }
        }
        
        xSemaphoreGive(_MessageAccessMutex);
    }
}

MessageBase *LoraUtils::MyLastBroacast() {
    if (xSemaphoreTake(_MessageAccessMutex, portMAX_DELAY) == pdTRUE) {
        MessageBase *msg = nullptr;

        if (_MyLastBroadcast != nullptr) {
            msg = _MyLastBroadcast->clone();
        }
        xSemaphoreGive(_MessageAccessMutex);

        return msg;
    }

    return nullptr;
}

MessageBase *LoraUtils::ReceivedMessage(uint64_t userID) {
    if (xSemaphoreTake(_MessageAccessMutex, portMAX_DELAY) == pdTRUE) {
        MessageBase *msg = _ReceivedMessages[userID]->clone();
        xSemaphoreGive(_MessageAccessMutex);
        return msg;
    }

    return nullptr;
}

void LoraUtils::SetMyLastBroadcast(MessageBase *msg) {
    if (xSemaphoreTake(_MessageAccessMutex, portMAX_DELAY) == pdTRUE) {
        if (_MyLastBroadcast != nullptr && _MyLastBroadcast != msg) {
            delete _MyLastBroadcast;
        }
        _MyLastBroadcast = msg->clone();
        xSemaphoreGive(_MessageAccessMutex);
    }
}

void LoraUtils::SetReceivedMessage(uint64_t userID, MessageBase *msg) {
    if (xSemaphoreTake(_MessageAccessMutex, portMAX_DELAY) == pdTRUE) {
        bool isMsgNew = true;

        if (_ReceivedMessages.find(userID) != _ReceivedMessages.end())
        {
            if (_ReceivedMessages[userID]->msgID == msg->msgID)
            {
                isMsgNew = false;
            }

            delete _ReceivedMessages[userID];
        }
        _ReceivedMessages[userID] = msg->clone();

        if (isMsgNew) 
        {
            if (_UnreadMessages.find(userID) != _UnreadMessages.end()) 
            {
                delete _UnreadMessages[userID];
            }

            _UnreadMessages[userID] = msg->clone();
        }
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
    auto msgType = MessageBase::GetMessageTypeFromMsgPackBuffer(buffer);

    if (_deserializers.find(msgType) == _deserializers.end())
    {
        return nullptr;
    }

    return _deserializers[msgType](buffer, len);
}

MessageBase *LoraUtils::DeserializeMessage(JsonDocument &jsonDoc)
{
    auto msgType = MessageBase::GetMessageTypeFromJson(jsonDoc);

    if (_deserializers.find(msgType) == _deserializers.end())
    {
        return nullptr;
    }

    auto bufferSize = measureMsgPack(jsonDoc) + 1;
    uint8_t buffer[bufferSize];
    serializeMsgPack(jsonDoc, buffer, bufferSize);

    return _deserializers[msgType](buffer, bufferSize);
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

bool LoraUtils::MessagePackSanityCheck(JsonDocument &doc)
{
    uint8_t buffer[measureMsgPack(doc)];
    serializeMsgPack(doc, buffer, sizeof(buffer));
    StaticJsonDocument<MSG_BASE_SIZE> doc2;
    auto returnCode = deserializeMsgPack(doc2, buffer, sizeof(buffer));
    if (returnCode != DeserializationError::Ok)
    {
        #if DEBUG == 1
        Serial.print("LoraUtils::MessagePackSanityCheck: Deserialization error: ");
        Serial.println(returnCode.c_str());
        #endif
        return false;
    }

    #if DEBUG == 1
    Serial.print("Doc1: ");
    serializeJson(doc, Serial);
    Serial.println();
    Serial.print("Doc2: ");
    serializeJson(doc2, Serial);
    Serial.println();
    Serial.println("Raw buffer:");
    for (size_t i = 0; i < sizeof(buffer); i++)
    {
        Serial.print(buffer[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
    #endif

    bool result = true;

    for (auto kv : doc.as<JsonObject>())
    {
        if (!doc2.containsKey(kv.key()))
        {
            #if DEBUG == 1
            Serial.print("LoraUtils::MessagePackSanityCheck: Key not found: ");
            Serial.println(kv.key().c_str());
            #endif
            result = false;
            continue;
        }

        if (doc2[kv.key()] != kv.value())
        {
            #if DEBUG == 1
            Serial.print("LoraUtils::MessagePackSanityCheck: Value mismatch for key: ");
            Serial.println(kv.key().c_str());
            Serial.printf("Expected: %s\n", kv.value().as<std::string>());
            Serial.printf("Actual: %s\n", doc2[kv.key()].as<std::string>());
            #endif
            result = false;
        }
    }

    return result;
}

void LoraUtils::AddSavedMessage(std::string msg, bool flash)
{
    _SavedMessageList.push_back(msg);

    if (flash)
        _SavedMessageListUpdated.Invoke();
}

void LoraUtils::DeleteSavedMessage(std::vector<std::string>::iterator &it)
{
    it = _SavedMessageList.erase(it);

    _SavedMessageListUpdated.Invoke();
}

void LoraUtils::ClearSavedMessages()
{
    _SavedMessageList.clear();

    _SavedMessageList.push_back("Meet here");
    _SavedMessageList.push_back("Point of interest");

    _SavedMessageListUpdated.Invoke();
}

void LoraUtils::UpdateSavedMessage(std::vector<std::string>::iterator &it, std::string msg)
{
    *it = msg;
}

void LoraUtils::SerializeSavedMessageList(JsonDocument &doc)
{
    JsonArray msgArray;

    if (doc.containsKey("messages"))
    {
        msgArray = doc["messages"].as<JsonArray>();
    }
    else
    {
        msgArray = doc.createNestedArray("messages");
    }

    for (auto msg : _SavedMessageList)
    {
        msgArray.add(msg);
    }
}   

void LoraUtils::DeserializeSavedMessageList(JsonDocument &doc)
{
    auto msgArray = doc["messages"].as<JsonArray>();
    _SavedMessageList.clear();

    for (auto msg : msgArray)
    {
        _SavedMessageList.push_back(msg.as<std::string>());
    }
}

void LoraUtils::AddUserInfo(UserInfo userInfo)
{
    _UserInfoList.push_back(userInfo);

    _UserInfoListUpdated.Invoke();
}

void LoraUtils::DeleteUserInfo(std::vector<UserInfo>::iterator &it)
{
    it = _UserInfoList.erase(it);

    _UserInfoListUpdated.Invoke();
}

void LoraUtils::UpdateUserInfo(std::vector<UserInfo>::iterator &it, UserInfo userInfo)
{
    it->Name = userInfo.Name;
    it->UserID = userInfo.UserID;
    
    _UserInfoListUpdated.Invoke();
}

void LoraUtils::SerializeUserInfoList(JsonDocument &doc)
{
    auto userArray = doc.createNestedArray("users");

    for (auto kv : _UserInfoList)
    {
        auto userObj = userArray.createNestedObject();

        userObj["userID"] = kv.UserID;
        userObj["name"] = kv.Name;
    }
}

void LoraUtils::DeserializeUserInfoList(JsonDocument &doc)
{
    auto userArray = doc["users"].as<JsonArray>();
    _UserInfoList.clear();

    for (auto userObj : userArray)
    {
        UserInfo user;

        user.UserID = userObj["userID"].as<uint32_t>();
        user.Name = userObj["name"].as<std::string>();
        _UserInfoList.push_back(user);
    }
}

void LoraUtils::FlashDefaultMessages()
{
    _SavedMessageList.clear();
    _SavedMessageList.push_back("Meet here");
    _SavedMessageList.push_back("Point of interest");
    _SavedMessageListUpdated.Invoke();
}