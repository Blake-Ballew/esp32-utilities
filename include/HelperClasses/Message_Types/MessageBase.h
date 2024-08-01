#pragma once

#include "globalDefines.h"
#include <ArduinoJson.h>
#include "LED_Manager.h"
#include "Navigation_Manager.h"
#include "Adafruit_SSD1306.h"

#define MSG_TYPE_OFFSET 0

#define MSG_BASE_SIZE 512

#define NAME_LENGTH 12
#define STATUS_LENGTH 32

namespace
{
    const char *MESSAGE_TYPE_KEY PROGMEM = "t";
    const char *MESSAGE_TYPE_ID PROGMEM = "i";
    const char *MESSAGE_TYPE_BOUNCES_LEFT PROGMEM = "B";
    const char *MESSAGE_TYPE_RECIPIENT PROGMEM = "R ";
    const char *MESSAGE_TYPE_FROM PROGMEM = "f";
    const char *MESSAGE_TYPE_FROM_NAME PROGMEM = "n";
    const char *MESSAGE_TYPE_TIME PROGMEM = "T";
    const char *MESSAGE_TYPE_DATE PROGMEM = "D";
    const char *MESSAGE_TYPE_COLOR_R PROGMEM = "r";
    const char *MESSAGE_TYPE_COLOR_G PROGMEM = "g";
    const char *MESSAGE_TYPE_COLOR_B PROGMEM = "b";
    const char *MESSAGE_TYPE_LAT PROGMEM = "a";
    const char *MESSAGE_TYPE_LNG PROGMEM = "o";
    const char *MESSAGE_TYPE_STATUS PROGMEM = "s";
}

// enum MessageType
// {
//     MESSAGE_INVALID = 0,
//     MESSAGE_BASE = 1,
//     MESSAGE_PING = 2
// };

// Base class for all LoRa messages
// All message classes should contain a static method to determine the message type
// and a static method to clone the message
class MessageBase
{
public:
    bool messageOpened;

    uint32_t msgID;
    uint8_t bouncesLeft;

    uint64_t recipient = 0;

    uint64_t sender;
    char senderName[NAME_LENGTH + 1];

    uint32_t time;
    uint32_t date;

    bool isValid;

    MessageBase()
    {
    }

    // Constructor for message created by this node
    MessageBase(uint32_t time, uint32_t date, uint64_t recipient, uint64_t sender, const char *senderName, uint32_t msgID)
    {
        messageOpened = false;
        isValid = true;

        this->time = time;
        this->date = date;
        this->recipient = recipient;
        this->sender = sender;
        this->bouncesLeft = 5;

        memcpy(this->senderName, senderName, NAME_LENGTH);
        if (msgID != 0)
        {
            this->msgID = msgID;
        }
        else
        {
            this->msgID = esp_random();
        }
    }
    // Constructor for message received from another node
    MessageBase(uint8_t *buffer)
    {
        messageOpened = false;
        deserialize(buffer);
    }

    virtual ~MessageBase()
    {
    }

    static uint8_t GetMessageTypeFromMsgPackBuffer(const uint8_t *buffer)
    {
        if (buffer == nullptr)
            return 0;
        ArduinoJson::StaticJsonDocument<MSG_BASE_SIZE> doc;
        
        if (deserializeMsgPack(doc, buffer, MSG_BASE_SIZE) != DeserializationError::Ok)
        {
            return 0;
        }

        if (doc.containsKey(MESSAGE_TYPE_KEY))
        {
            return doc[MESSAGE_TYPE_KEY].as<uint8_t>();
        }
        else
        {
            return 0;
        }
    }

    static uint8_t GetMessageTypeFromJson(JsonObject &doc)
    {
        if (doc.containsKey(MESSAGE_TYPE_KEY))
        {
            return doc[MESSAGE_TYPE_KEY].as<uint8_t>();
        }
        else
        {
            return 0;
        }
    }

    virtual uint8_t *serialize(size_t &len)
    {
        ArduinoJson::StaticJsonDocument<MSG_BASE_SIZE> doc;
        doc[MESSAGE_TYPE_KEY] = _MessageType;
        doc[MESSAGE_TYPE_ID] = msgID;
        doc[MESSAGE_TYPE_BOUNCES_LEFT] = bouncesLeft;
        doc[MESSAGE_TYPE_RECIPIENT] = recipient;
        doc[MESSAGE_TYPE_FROM] = sender;
        doc[MESSAGE_TYPE_FROM_NAME] = senderName;
        doc[MESSAGE_TYPE_TIME] = time;
        doc[MESSAGE_TYPE_DATE] = date;

        len = measureMsgPack(doc) + 1;
        uint8_t *buffer = new uint8_t[len];
        serializeMsgPack(doc, buffer, len);
        buffer[len - 1] = '\0';
        return buffer;
    }

    virtual void deserialize(const uint8_t *buffer)
    {
        ArduinoJson::StaticJsonDocument<MSG_BASE_SIZE> doc;

        deserializeMsgPack(doc, buffer, MSG_BASE_SIZE);

        auto msgType = doc[MESSAGE_TYPE_KEY].as<uint8_t>();
        if (msgType != _MessageType)
        {
            isValid = false;
            return;
        }

        isValid = true;
        msgID = doc[MESSAGE_TYPE_ID];
        if (doc.containsKey(MESSAGE_TYPE_BOUNCES_LEFT))
            bouncesLeft = doc[MESSAGE_TYPE_BOUNCES_LEFT];
        else
            bouncesLeft = 0;
        if (doc.containsKey(MESSAGE_TYPE_RECIPIENT))
            recipient = doc[MESSAGE_TYPE_RECIPIENT];
        sender = doc[MESSAGE_TYPE_FROM];
        strcpy(senderName, doc[MESSAGE_TYPE_FROM_NAME].as<const char *>());
        time = doc[MESSAGE_TYPE_TIME];
        date = doc[MESSAGE_TYPE_DATE];
    }

    virtual void deserialize(JsonObject &doc)
    {
        auto msgType = doc[MESSAGE_TYPE_KEY].as<uint8_t>();
        if (msgType != _MessageType)
        {
            isValid = false;
            return;
        }

        isValid = true;
        msgID = doc[MESSAGE_TYPE_ID];
        if (doc.containsKey(MESSAGE_TYPE_BOUNCES_LEFT))
            bouncesLeft = doc[MESSAGE_TYPE_BOUNCES_LEFT];
        else
            bouncesLeft = 0;
        if (doc.containsKey(MESSAGE_TYPE_RECIPIENT))
            recipient = doc[MESSAGE_TYPE_RECIPIENT];
        sender = doc[MESSAGE_TYPE_FROM];
        strcpy(senderName, doc[MESSAGE_TYPE_FROM_NAME].as<const char *>());
        time = doc[MESSAGE_TYPE_TIME];
        date = doc[MESSAGE_TYPE_DATE];
    }

    virtual DynamicJsonDocument *serializeJSON()
    {
        DynamicJsonDocument *doc = new DynamicJsonDocument(MSG_BASE_SIZE);
        (*doc)[MESSAGE_TYPE_KEY] = _MessageType;
        (*doc)[MESSAGE_TYPE_ID] = msgID;
        (*doc)[MESSAGE_TYPE_BOUNCES_LEFT] = bouncesLeft;
        (*doc)[MESSAGE_TYPE_RECIPIENT] = recipient;
        (*doc)[MESSAGE_TYPE_FROM] = sender;
        (*doc)[MESSAGE_TYPE_FROM_NAME] = senderName;
        (*doc)[MESSAGE_TYPE_TIME] = time;
        (*doc)[MESSAGE_TYPE_DATE] = date;
        return doc;
    }

    virtual MessageBase *clone()
    {
        return new MessageBase(time, date, recipient, sender, senderName, msgID);
    }

    virtual void toString(char *buffer, size_t bufferLen)
    {
        snprintf(buffer, bufferLen, "MsgID: %X", msgID);
    }

    static void printFromBuffer(uint8_t *buffer)
    {
        ArduinoJson::StaticJsonDocument<MSG_BASE_SIZE> doc;
        deserializeMsgPack(doc, buffer, MSG_BASE_SIZE);
        serializeJson(doc, Serial);
    }

    virtual void displayMessage(Adafruit_SSD1306 *display)
    {
        display->setCursor(0, 8);
        display->print(this->senderName);

        display->setCursor(0, 16);
        display->print(F("MsgID: "));
        display->print(this->msgID, HEX);

        display->setCursor(110, 8);
        printMessageAge(Navigation_Manager::getTimeDifference(this->time, this->date), display);
    }

    static void printMessageAge(uint64_t timeDiff, Adafruit_SSD1306 *display)
    {
        uint8_t diffHours = (timeDiff & 0xFF000000) >> 24;
        uint8_t diffMinutes = (timeDiff & 0xFF0000) >> 16;

#if DEBUG == 1
        Serial.print("Time diff: ");
        Serial.println(timeDiff);
        Serial.print("Hours: ");
        Serial.println(diffHours);
        Serial.print("Minutes: ");
        Serial.println(diffMinutes);
#endif

        if (timeDiff > 0xFFFFFFFF)
        {
            display->print(">1d");
        }
        else if (diffHours > 0)
        {
            display->print(diffHours);
            display->print(F("h"));
        }
        else if (diffMinutes > 0)
        {
            display->print(diffMinutes);
            display->print(F("m"));
        }
        else
        {
            display->print(F("<1m"));
        }
    }

    static MessageBase *MessageFactory(uint8_t *buffer, size_t len) 
    {
        MessageBase *msg = new MessageBase(buffer);
        
        if (msg->isValid)
        {
            return msg;
        }
        else
        {
            delete msg;
            return nullptr;
        }
    }

    static uint8_t MessageType()
    {
        return _MessageType;
    }

    static void SetMessageType(uint8_t type)
    {
        _MessageType = type;
    }

protected:
    static uint8_t _MessageType;
};

