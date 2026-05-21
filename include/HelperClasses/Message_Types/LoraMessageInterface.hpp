#pragma once

#include <ArduinoJson.h>
#include <string>
#include <vector>
#include <algorithm>
#include <memory>

#define MSG_BASE_SIZE 512
#define NAME_LENGTH 12

namespace
{
    const size_t MAX_LEN_MESSAGE_PRINT_INFO = 64;
}

struct MessagePrintInformation
{
    char txt[MAX_LEN_MESSAGE_PRINT_INFO];

    MessagePrintInformation(const char* txt)
    {
        strncpy(this->txt, txt, MAX_LEN_MESSAGE_PRINT_INFO);
        this->txt[MAX_LEN_MESSAGE_PRINT_INFO - 1] = '\0';
    }

    MessagePrintInformation(const MessagePrintInformation& mpi)
    {
        strncpy(this->txt, mpi.txt, MAX_LEN_MESSAGE_PRINT_INFO);
    }
};

namespace LoraModule
{
    // FNV-1a 32-bit hash over sorted required payload field key chars.
    // constexpr — usable at compile time for static GUID members.
    inline uint32_t schemaHash(const std::string& keys)
    {
        uint32_t h = 0x811C9DC5u;
        for (char c : keys)
        {
            h ^= static_cast<uint8_t>(c);
            h *= 0x01000193u;
        }
        return h;
    }

    // Collect key names from a received payload JsonObject, sort them,
    // concatenate, and hash to produce the schema GUID.
    inline uint32_t computeSchemaGuid(JsonObject& payload)
    {
        std::vector<std::string> keys;
        keys.reserve(payload.size());
        for (auto kv : payload) { keys.emplace_back(kv.key().c_str()); }
        std::sort(keys.begin(), keys.end());

        std::string combined;
        for (const auto& k : keys) { combined += k; }
        return schemaHash(combined.c_str());
    }

    class LoraMessageInterface
    {
    public:
        static constexpr const char* TAG            = "LoraMessage";
        static constexpr const char* KEY_PAYLOAD      = "p";
        static constexpr const char* KEY_MSG_ID       = "i";
        static constexpr const char* KEY_BOUNCES_LEFT = "B";
        static constexpr const char* KEY_FROM         = "f";
        static constexpr const char* KEY_TIME         = "T";
        static constexpr const char* KEY_DATE         = "D";

        uint32_t msgID       = 0;
        uint8_t  bouncesLeft = 0;
        uint32_t sender      = 0;
        uint32_t time        = 0;
        uint32_t date        = 0;

        // Writes base routing fields to root and calls serializePayload() into doc["p"].
        virtual bool serialize(JsonDocument& doc)
        {
            doc[KEY_MSG_ID]       = msgID;
            doc[KEY_BOUNCES_LEFT] = bouncesLeft;
            doc[KEY_FROM]         = sender;
            doc[KEY_TIME]         = time;
            doc[KEY_DATE]         = date;

            JsonObject payload = doc.createNestedObject(KEY_PAYLOAD);
            if (!serializePayload(payload)) { return false; }

            return !doc.overflowed();
        }

        // Fills only the base routing fields from root. Payload is handled separately.
        virtual void deserialize(JsonDocument& doc)
        {
            msgID       = doc[KEY_MSG_ID]       | 0u;
            bouncesLeft = doc[KEY_BOUNCES_LEFT]  | uint8_t(0);
            sender      = doc[KEY_FROM]          | 0u;
            time        = doc[KEY_TIME]          | 0u;
            date        = doc[KEY_DATE]          | 0u;
        }

        virtual bool serializePayload(JsonObject& payload) = 0;
        virtual void deserializePayload(JsonObject& payload) = 0;

        // 32-bit FNV-1a hash of sorted required payload field key chars.
        // Used for type dispatch; never transmitted.
        virtual uint32_t SchemaGuid() const = 0;

        virtual std::shared_ptr<LoraMessageInterface> clone() const = 0;

        virtual bool IsValid() const { return msgID != 0 && sender != 0; }

        virtual void GetPrintableInformation(std::vector<MessagePrintInformation>&) = 0;

        virtual ~LoraMessageInterface() = default;

        static std::string GetMessageAge(uint64_t timeDiff)
        {
            uint8_t diffHours   = (timeDiff & 0xFF000000) >> 24;
            uint8_t diffMinutes = (timeDiff & 0xFF0000)   >> 16;

            ESP_LOGV(TAG, "Time diff: %llu, Hours: %d, Minutes: %d", timeDiff, diffHours, diffMinutes);

            if (timeDiff > 0xFFFFFFFF) { return ">1d"; }
            if (diffHours > 0)         { return std::to_string(diffHours)   + "h"; }
            if (diffMinutes > 0)       { return std::to_string(diffMinutes) + "m"; }
            return "<1m";
        }
    };
}
