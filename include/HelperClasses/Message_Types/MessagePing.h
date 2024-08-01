#pragma once

#include <MessageBase.h>

class MessagePing : public MessageBase
{
public:
    MessagePing()
    {
    }

    MessagePing(uint32_t time, uint32_t date, uint64_t recipient, uint64_t sender, const char *senderName, uint32_t msgID, uint8_t color_R, uint8_t color_G, uint8_t color_B, double lat, double lng, const char *status)
        : MessageBase(time, date, recipient, sender, senderName, msgID)
    {
        this->isValid = true;
        messageOpened = false;

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

    MessagePing(uint8_t *buffer)
    {
        messageOpened = false;
        deserialize(buffer);
    }

    ~MessagePing()
    {
    }

    uint8_t *serialize(size_t &len)
    {
        ArduinoJson::StaticJsonDocument<MSG_BASE_SIZE> doc;
        doc[MESSAGE_TYPE_KEY] = _MessageType;
        doc[MESSAGE_TYPE_ID] = msgID;
        doc[MESSAGE_TYPE_RECIPIENT] = recipient;
        doc[MESSAGE_TYPE_FROM] = sender;
        doc[MESSAGE_TYPE_FROM_NAME] = senderName;
        doc[MESSAGE_TYPE_TIME] = time;
        doc[MESSAGE_TYPE_DATE] = date;

        doc[MESSAGE_TYPE_LAT] = lat;
        doc[MESSAGE_TYPE_LNG] = lng;

        doc[MESSAGE_TYPE_STATUS] = status;

        doc[MESSAGE_TYPE_COLOR_R] = color_R;
        doc[MESSAGE_TYPE_COLOR_G] = color_G;
        doc[MESSAGE_TYPE_COLOR_B] = color_B;

        len = measureMsgPack(doc);
        uint8_t *buffer = new uint8_t[len];
        serializeMsgPack(doc, buffer, len);

#if DEBUG == 1
        Serial.print("Ping msg length: ");
        Serial.println(len);
        serializeJson(doc, Serial);
        Serial.println();
#endif
        return buffer;
    }

    void deserialize(uint8_t *buffer)
    {
        ArduinoJson::StaticJsonDocument<MSG_BASE_SIZE> doc;

        deserializeMsgPack(doc, buffer, MSG_BASE_SIZE);
#if DEBUG == 1
        serializeJson(doc, Serial);
        Serial.println();
#endif

        auto msgType = doc[MESSAGE_TYPE_KEY].as<uint8_t>();
        if (msgType != _MessageType)
        {
            isValid = false;
            return;
        }

        isValid = true;

        msgID = doc[MESSAGE_TYPE_ID];
        if (doc.containsKey(MESSAGE_TYPE_RECIPIENT))
            recipient = doc[MESSAGE_TYPE_RECIPIENT];
        sender = doc[MESSAGE_TYPE_FROM];
        const char *sendName = doc[MESSAGE_TYPE_FROM_NAME].as<const char *>();
        size_t nameLen = strlen(sendName);
        if (nameLen > NAME_LENGTH)
            nameLen = NAME_LENGTH;
        strncpy(senderName, sendName, nameLen);
        senderName[nameLen] = '\0';
        time = doc[MESSAGE_TYPE_TIME];
        date = doc[MESSAGE_TYPE_DATE];

        lat = doc[MESSAGE_TYPE_LAT];
        lng = doc[MESSAGE_TYPE_LNG];

        const char *msgStatus = doc[MESSAGE_TYPE_STATUS].as<const char *>();
        size_t strLen = 0;
        if (msgStatus == nullptr)
        {
#if DEBUG == 1
            Serial.println("Status is null");
#endif
        }
        else
        {
            strLen = strlen(msgStatus);
            Serial.print("Status length: ");
            Serial.println(strLen);
        }

        if (strLen > STATUS_LENGTH)
        {
#if DEBUG == 1
            Serial.println("Status length is too long");
#endif
            strLen = STATUS_LENGTH;
        }

        if (msgStatus != nullptr)
        {
            strncpy(status, msgStatus, strLen);
            status[strLen] = '\0';
        }

        color_R = doc[MESSAGE_TYPE_COLOR_R];
        color_G = doc[MESSAGE_TYPE_COLOR_G];
        color_B = doc[MESSAGE_TYPE_COLOR_B];
    }

    void deserialize(JsonObject &doc)
    {
        auto msgType = doc[MESSAGE_TYPE_KEY].as<uint8_t>();
        if (msgType != _MessageType)
        {
            isValid = false;
            return;
        }

        msgID = doc[MESSAGE_TYPE_ID];
        if (doc.containsKey(MESSAGE_TYPE_RECIPIENT))
            recipient = doc[MESSAGE_TYPE_RECIPIENT];
        sender = doc[MESSAGE_TYPE_FROM];
        strcpy(senderName, doc[MESSAGE_TYPE_FROM_NAME].as<const char *>());
        time = doc[MESSAGE_TYPE_TIME];
        date = doc[MESSAGE_TYPE_DATE];

        lat = doc[MESSAGE_TYPE_LAT];
        lng = doc[MESSAGE_TYPE_LNG];

        strcpy(status, doc[MESSAGE_TYPE_STATUS].as<const char *>());

        color_R = doc[MESSAGE_TYPE_COLOR_R];
        color_G = doc[MESSAGE_TYPE_COLOR_G];
        color_B = doc[MESSAGE_TYPE_COLOR_B];
    }

    DynamicJsonDocument *serializeJSON()
    {
        DynamicJsonDocument *doc = new DynamicJsonDocument(MSG_BASE_SIZE);
        (*doc)[MESSAGE_TYPE_KEY] = _MessageType;
        (*doc)[MESSAGE_TYPE_ID] = msgID;
        (*doc)[MESSAGE_TYPE_RECIPIENT] = recipient;
        (*doc)[MESSAGE_TYPE_FROM] = sender;
        (*doc)[MESSAGE_TYPE_FROM_NAME] = senderName;
        (*doc)[MESSAGE_TYPE_TIME] = time;
        (*doc)[MESSAGE_TYPE_DATE] = date;

        (*doc)[MESSAGE_TYPE_LAT] = lat;
        (*doc)[MESSAGE_TYPE_LNG] = lng;

        (*doc)[MESSAGE_TYPE_STATUS] = status;

        (*doc)[MESSAGE_TYPE_COLOR_R] = color_R;
        (*doc)[MESSAGE_TYPE_COLOR_G] = color_G;
        (*doc)[MESSAGE_TYPE_COLOR_B] = color_B;

        return doc;
    }

    MessageBase *clone()
    {
        return new MessagePing(time, date, recipient, sender, senderName, msgID, color_R, color_G, color_B, lat, lng, status);
    }

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

    void toString(char *buffer, size_t bufferLen)
    {
        snprintf(buffer, bufferLen, status);
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
        MessageBase *msg = new MessagePing(buffer);
        if (!msg->isValid)
        {
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