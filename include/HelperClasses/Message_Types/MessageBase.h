#pragma once

#include "globalDefines.h"
#include <ArduinoJson.h>
#include "LED_Manager.h"
#include "NavigationUtils.h"
#include "Adafruit_SSD1306.h"
#include <string>

#define MSG_TYPE_OFFSET 0

#define MSG_BASE_SIZE 512

#define NAME_LENGTH 12

namespace
{
    const char *MESSAGE_TYPE_KEY PROGMEM = "t";
    const char *MESSAGE_TYPE_ID PROGMEM = "i";
    const char *MESSAGE_TYPE_BOUNCES_LEFT PROGMEM = "B";
    const char *MESSAGE_TYPE_RECIPIENT PROGMEM = "R";
    const char *MESSAGE_TYPE_FROM PROGMEM = "f";
    const char *MESSAGE_TYPE_FROM_NAME PROGMEM = "n";
    const char *MESSAGE_TYPE_TIME PROGMEM = "T";
    const char *MESSAGE_TYPE_DATE PROGMEM = "D";

    const size_t MAX_LEN_MESSAGE_PRINT_INFO = 64;
}

struct MessagePrintInformation
{
    char txt[MAX_LEN_MESSAGE_PRINT_INFO];

    MessagePrintInformation(const char *txt)
    {
        strncpy(this->txt, txt, MAX_LEN_MESSAGE_PRINT_INFO);
    }

    // Copy constructor
    MessagePrintInformation(const MessagePrintInformation &mpi)
    {
        strncpy(this->txt, mpi.txt, MAX_LEN_MESSAGE_PRINT_INFO);
    }
};

// Base class for all LoRa messages
// All message classes should contain a static method to determine the message type
// and a static method to clone the message
class MessageBase
{
public:
    // Unique ID for the message
    uint32_t msgID;

    // Number of times the message can be forwarded. Each node decrements this value
    uint8_t bouncesLeft;

    // ID of the recipient. 0 is broadcast
    uint32_t recipient = 0;

    // ID of the sender. This should never be 0
    uint32_t sender;

    // Name of the sender
    char senderName[NAME_LENGTH + 1];

    // Time and date the message was sent
    uint32_t time;
    uint32_t date;

    MessageBase()
    {
    }

    // Constructor for message created by this node
    MessageBase(uint32_t time, uint32_t date, uint32_t recipient, uint32_t sender, const char *senderName, uint32_t msgID)
    {
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

    static uint8_t GetMessageTypeFromJson(JsonDocument &doc)
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

    virtual bool serialize(JsonDocument &doc)
    {
        std::string senderNameStr(senderName);

        doc[MESSAGE_TYPE_KEY] = GetInstanceMessageType();
        doc[MESSAGE_TYPE_ID] = msgID;
        doc[MESSAGE_TYPE_BOUNCES_LEFT] = bouncesLeft;
        doc[MESSAGE_TYPE_RECIPIENT] = recipient;
        doc[MESSAGE_TYPE_FROM] = sender;
        doc[MESSAGE_TYPE_FROM_NAME] = senderNameStr;
        doc[MESSAGE_TYPE_TIME] = time;
        doc[MESSAGE_TYPE_DATE] = date;

        if (doc.overflowed())
        {
            return false;
        }

        return true;
    }

    virtual void deserialize(JsonDocument &doc)
    {
        if (doc.containsKey(MESSAGE_TYPE_ID))
        {
            msgID = doc[MESSAGE_TYPE_ID].as<uint32_t>();
        }
        else
        {
            #if DEBUG == 1 
            Serial.println("MessageBase::deserialize: No message ID found");
            #endif
            msgID = 0;
        }

        if (doc.containsKey(MESSAGE_TYPE_BOUNCES_LEFT))
        {
            bouncesLeft = doc[MESSAGE_TYPE_BOUNCES_LEFT].as<uint8_t>();
        }
        else
        {
            #if DEBUG == 1 
            Serial.println("MessageBase::deserialize: No bounces left found");
            #endif
            bouncesLeft = 0;
        }

        if (doc.containsKey(MESSAGE_TYPE_RECIPIENT))
        {
            recipient = doc[MESSAGE_TYPE_RECIPIENT].as<uint32_t>();
        }
        else
        {
            #if DEBUG == 1 
            Serial.println("MessageBase::deserialize: No recipient found");
            #endif
            recipient = 0;
        }

        if (doc.containsKey(MESSAGE_TYPE_FROM))
        {
            sender = doc[MESSAGE_TYPE_FROM].as<uint32_t>();
        }
        else
        {
            #if DEBUG == 1 
            Serial.println("MessageBase::deserialize: No sender found");
            #endif
            sender = 0;
        }

        const char *sendName = doc[MESSAGE_TYPE_FROM_NAME].as<const char *>();
        strncpy(senderName, sendName, NAME_LENGTH);

        if(doc.containsKey(MESSAGE_TYPE_TIME))
        {
            time = doc[MESSAGE_TYPE_TIME].as<uint32_t>();
        }
        else
        {
            #if DEBUG == 1 
            Serial.println("MessageBase::deserialize: No time found");
            #endif
            time = 0;
        }

        if(doc.containsKey(MESSAGE_TYPE_DATE))
        {
            date = doc[MESSAGE_TYPE_DATE].as<uint32_t>();
        }
        else
        {
            #if DEBUG == 1 
            Serial.println("MessageBase::deserialize: No date found");
            #endif
            date = 0;
        }
    }

    virtual MessageBase *clone()
    {
        MessageBase *newMsg = new MessageBase(time, date, recipient, sender, senderName, msgID);
        newMsg->bouncesLeft = bouncesLeft;
        return newMsg;
    }

    virtual void GetPrintableInformation(std::vector<MessagePrintInformation> &info)
    {
        MessagePrintInformation mpi(senderName);
        info.push_back(mpi);

        char buffer[32];
        snprintf(buffer, 32, "MsgID: %X", msgID);
        MessagePrintInformation mpi2(buffer);
        info.push_back(mpi2);
    }

    static void printFromBuffer(uint8_t *buffer)
    {
        ArduinoJson::StaticJsonDocument<MSG_BASE_SIZE> doc;
        deserializeMsgPack(doc, buffer, MSG_BASE_SIZE);
        serializeJson(doc, Serial);
    }

    // TODO: Get rid of this
    // virtual void displayMessage(Adafruit_SSD1306 *display)
    // {
    //     display->setCursor(0, 8);
    //     display->print(this->senderName);

    //     display->setCursor(0, 16);
    //     display->print(F("MsgID: "));
    //     display->print(this->msgID, HEX);

    //     display->setCursor(110, 8);
    //     printMessageAge(NavigationUtils::GetTimeDifference(this->time, this->date), display);
    // }

    virtual bool IsValid()
    {
        return (msgID != 0 && sender != 0);
    }

    static std::string GetMessageAge(uint64_t timeDiff)
    {
        uint8_t diffHours = (timeDiff & 0xFF000000) >> 24;
        uint8_t diffMinutes = (timeDiff & 0xFF0000) >> 16;

#if DEBUG == 1
        // Serial.print("Time diff: ");
        // Serial.println(timeDiff);
        // Serial.print("Hours: ");
        // Serial.println(diffHours);
        // Serial.print("Minutes: ");
        // Serial.println(diffMinutes);
#endif

        std::string ageStr;

        if (timeDiff > 0xFFFFFFFF)
        {
            ageStr = ">1d";
        }
        else if (diffHours > 0)
        {
            ageStr = std::to_string(diffHours) + "h";
        }    
        else if (diffMinutes > 0)
        {
            ageStr = std::to_string(diffMinutes) + "m";
        }
        else
        {
            ageStr = "<1m";
        }

        return ageStr;
    }

    static MessageBase *MessageFactory(uint8_t *buffer, size_t len) 
    {
        #if DEBUG == 1
        Serial.println("MessageBase::MessageFactory");
        #endif
        MessageBase *msg = new MessageBase();
        StaticJsonDocument<MSG_BASE_SIZE> doc;

        if (deserializeMsgPack(doc, (const char *)buffer, len) != DeserializationError::Ok)
        {
            delete msg;
            return nullptr;
        }

        msg->deserialize(doc);

        if (msg->IsValid())
        {
            return msg;
        }
        else
        {
            #if DEBUG == 1
            Serial.println("MessageBase::MessageFactory: Invalid message");
            #endif
            delete msg;
            return nullptr;
        }
    }

    virtual uint8_t GetInstanceMessageType()
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

protected:
    static uint8_t _MessageType;
};

