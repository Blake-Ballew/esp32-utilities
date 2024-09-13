#pragma once

#include <MessageBase.h>

namespace
{
    const char *MESSAGE_TYPE_COLOR_R PROGMEM = "r";
    const char *MESSAGE_TYPE_COLOR_G PROGMEM = "g";
    const char *MESSAGE_TYPE_COLOR_B PROGMEM = "b";
    const char *MESSAGE_TYPE_LAT PROGMEM = "a";
    const char *MESSAGE_TYPE_LNG PROGMEM = "o";
    const char *MESSAGE_TYPE_STATUS PROGMEM = "s";
    const char *MESSAGE_TYPE_IS_LIVE PROGMEM = "L";
}

const size_t STATUS_LENGTH = 23;

class MessagePing : public MessageBase
{
public:
    MessagePing() : MessageBase()
    {
        this->IsLive = false;
    }

    MessagePing(uint32_t time, uint32_t date, uint32_t recipient, uint32_t sender, const char *senderName, uint32_t msgID, uint8_t color_R, uint8_t color_G, uint8_t color_B, double lat, double lng, const char *status)
        : MessageBase(time, date, recipient, sender, senderName, msgID)
    {
        this->color_R = color_R;
        this->color_G = color_G;
        this->color_B = color_B;

        this->lat = lat;
        this->lng = lng;

        this->IsLive = false;

        size_t strLen = strlen(status);
        if (strLen > STATUS_LENGTH)
            strLen = STATUS_LENGTH;

        memcpy(this->status, status, strLen);
        this->status[strLen] = '\0';
    }

    ~MessagePing()
    {
    }

    bool serialize(JsonDocument &doc)
    {
        if (!MessageBase::serialize(doc))
        {
            return false;
        }

        std::string statusStr(this->status);

        doc[MESSAGE_TYPE_COLOR_R] = this->color_R;
        doc[MESSAGE_TYPE_COLOR_G] = this->color_G;
        doc[MESSAGE_TYPE_COLOR_B] = this->color_B;
        doc[MESSAGE_TYPE_LAT] = this->lat;
        doc[MESSAGE_TYPE_LNG] = this->lng;
        doc[MESSAGE_TYPE_STATUS] = statusStr;
        doc[MESSAGE_TYPE_IS_LIVE] = this->IsLive;

        if (doc.overflowed())
        {
            return false;
        }

        return true;
    }

    void deserialize(JsonDocument &doc)
    {
        MessageBase::deserialize(doc);

        if (doc.containsKey(MESSAGE_TYPE_COLOR_R))
            color_R = doc[MESSAGE_TYPE_COLOR_R];
        else
            color_R = 0;

        if (doc.containsKey(MESSAGE_TYPE_COLOR_G))
            color_G = doc[MESSAGE_TYPE_COLOR_G];
        else
            color_G = 0;

        if (doc.containsKey(MESSAGE_TYPE_COLOR_B)) 
            color_B = doc[MESSAGE_TYPE_COLOR_B];
        else
            color_B = 0;

        if (doc.containsKey(MESSAGE_TYPE_LAT))
            lat = doc[MESSAGE_TYPE_LAT];
        else
            lat = 0;

        if (doc.containsKey(MESSAGE_TYPE_LNG))
            lng = doc[MESSAGE_TYPE_LNG];
        else
            lng = 0;

        if (doc.containsKey(MESSAGE_TYPE_IS_LIVE))
            IsLive = doc[MESSAGE_TYPE_IS_LIVE];
        else
            IsLive = false;

        
        strncpy(status, doc[MESSAGE_TYPE_STATUS], STATUS_LENGTH);
        status[STATUS_LENGTH] = '\0';
    }

    MessageBase *clone() override
    {
        MessagePing *newMsg = new MessagePing(time, date, recipient, sender, senderName, msgID, color_R, color_G, color_B, lat, lng, status);
        newMsg->bouncesLeft = bouncesLeft;
        newMsg->IsLive = this->IsLive;
        return newMsg;
    }

    void GetPrintableInformation(std::vector<MessagePrintInformation> &info)
    {
        MessagePrintInformation mpi(senderName);
        info.push_back(mpi);

        MessagePrintInformation mpi2(this->status);
        info.push_back(mpi2);
    }

    // TODO: Get rid of this
    // void toString(char *buffer, size_t bufferLen)
    // {
    //     snprintf(buffer, bufferLen, status);
    // }

    uint8_t GetInstanceMessageType()
    {
        return _MessageType;
    }

    static uint8_t MessageType()
    {
        return _MessageType;
    }

    static void SetMessageType(uint8_t type)
    {
        _MessageType = type;
    }

    static MessageBase *MessageFactory(uint8_t *buffer, size_t len)
    {
        #if DEBUG == 1
        // Serial.println("MessagePing::MessageFactory");
        #endif
        MessageBase *msg = new MessagePing();
        StaticJsonDocument<MSG_BASE_SIZE> doc;

        if (deserializeMsgPack(doc, (const char *)buffer, len) != DeserializationError::Ok)
        {
            delete msg;
            return nullptr;
        }

        msg->deserialize(doc);

        if (!msg->IsValid())
        {
            #if DEBUG == 1
            Serial.println("MessagePing::MessageFactory: Invalid message");
            #endif
            delete msg;
            return nullptr;
        }

        return msg;
    }

    uint8_t color_R;
    uint8_t color_G;
    uint8_t color_B;

    double lat;
    double lng;

    char status[STATUS_LENGTH + 1];

    // Flag that indicates if the message is a live location
    bool IsLive;

protected:
    static uint8_t _MessageType;
};