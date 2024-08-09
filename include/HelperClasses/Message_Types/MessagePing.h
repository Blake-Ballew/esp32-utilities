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
}

class MessagePing : public MessageBase
{
public:
    MessagePing() : MessageBase()
    {
        
    }

    MessagePing(uint32_t time, uint32_t date, uint32_t recipient, uint32_t sender, const char *senderName, uint32_t msgID, uint8_t color_R, uint8_t color_G, uint8_t color_B, double lat, double lng, const char *status)
        : MessageBase(time, date, recipient, sender, senderName, msgID)
    {
        this->color_R = color_R;
        this->color_G = color_G;
        this->color_B = color_B;

        this->lat = lat;
        this->lng = lng;

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

        std::string statusStr(status);

        doc[MESSAGE_TYPE_COLOR_R] = color_R;
        doc[MESSAGE_TYPE_COLOR_G] = color_G;
        doc[MESSAGE_TYPE_COLOR_B] = color_B;
        doc[MESSAGE_TYPE_LAT] = lat;
        doc[MESSAGE_TYPE_LNG] = lng;
        doc[MESSAGE_TYPE_STATUS] = statusStr;

        if (doc.overflowed())
        {
            return false;
        }

        return true;
    }

    void deserialize(JsonDocument &doc)
    {
        MessageBase::deserialize(doc);

        color_R = doc[MESSAGE_TYPE_COLOR_R] | 0;
        color_G = doc[MESSAGE_TYPE_COLOR_G] | 0;
        color_B = doc[MESSAGE_TYPE_COLOR_B] | 255;

        lat = doc[MESSAGE_TYPE_LAT] | 0;
        lng = doc[MESSAGE_TYPE_LNG] | 0;
        strncpy(status, doc[MESSAGE_TYPE_STATUS], STATUS_LENGTH);
        status[STATUS_LENGTH] = '\0';
    }

    MessageBase *clone()
    {
        MessagePing *newMsg = new MessagePing(time, date, recipient, sender, senderName, msgID, color_R, color_G, color_B, lat, lng, status);
        newMsg->bouncesLeft = bouncesLeft;
        newMsg->isValid = isValid;
        return newMsg;
    }

    // TODO: Get rid of this
    void displayMessage(Adafruit_SSD1306 *display)
    {
        Navigation_Manager::updateGPS();

        uint64_t timeDiff = Navigation_Manager::getTimeDifference(this->time, this->date);

        display->setCursor(110, 8);
        // 128display->print(F("Recv: "));

        // Greater than one day
        printMessageAge(timeDiff, display);

        display->setCursor(0, 8);
        display->print(this->senderName);

        display->setCursor(0, 16);
        display->print(this->status);

        LED_Manager::lightRing(this->color_R, this->color_G, this->color_B);
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
        Serial.println("MessagePing::MessageFactory");
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

protected:
    static uint8_t _MessageType;
};