#pragma once

#include <ArduinoJson.h>
#include <cstring>
#include <map>
#include <unordered_map>
#include <vector>
#include <memory>
#include <string>

#include "LoraMessageInterface.hpp"
#include "EncryptionUtils.hpp"
#include "EventHandler.h"
#include "System_Utils.h"
#include "SettingsInterface.hpp"

namespace LoraModule
{
    using MessageCreator = std::shared_ptr<LoraMessageInterface>(*)(JsonObject& payload);

    namespace
    {
        const size_t LORA_MESSAGE_QUEUE_LENGTH  = 8;
        const uint8_t LORA_DEFAULT_SEND_ATTEMPTS = 3;
    }

    class Utilities
    {
    public:
        static constexpr const char* TAG = "LoraUtils";

        static void Init()
        {
            if (MessageSendQueueID() != -1) { return; }

            static StaticQueue_t queueBuffer;
            static uint8_t queueStorage[LORA_MESSAGE_QUEUE_LENGTH * sizeof(std::shared_ptr<LoraMessageInterface>*)];
            MessageSendQueueID() = System_Utils::registerQueue(
                LORA_MESSAGE_QUEUE_LENGTH,
                sizeof(std::shared_ptr<LoraMessageInterface>*),
                queueStorage,
                queueBuffer);
        }

        // Queues a message for sending. Clones the message internally.
        static bool SendMessage(std::shared_ptr<LoraMessageInterface> msg)
        {
            if (!msg) { return false; }
            if (MessageSendQueueID() == -1) { return false; }

            if (msg->sender == System_Utils::DeviceID)
            {
                SetMyLastBroadcast(msg);
            }

            auto* wrapper = new std::shared_ptr<LoraMessageInterface>(msg->clone());
            return System_Utils::sendToQueue(MessageSendQueueID(), &wrapper, 1000);
        }

        // Returns a shared_ptr to the last broadcast (shared ownership, no clone needed).
        static std::shared_ptr<LoraMessageInterface> MyLastBroadcast()
        {
            if (xSemaphoreTake(MessageAccessMutex(), portMAX_DELAY) == pdTRUE)
            {
                auto msg = MyLastBroadcastMsg();
                xSemaphoreGive(MessageAccessMutex());
                return msg;
            }
            return nullptr;
        }

        static void SetMyLastBroadcast(std::shared_ptr<LoraMessageInterface> msg)
        {
            if (xSemaphoreTake(MessageAccessMutex(), portMAX_DELAY) == pdTRUE)
            {
                MyLastBroadcastMsg() = msg;
                xSemaphoreGive(MessageAccessMutex());
            }
            EchoCount() = 0;
            MyLastBroadcastChanged().Invoke();
        }

        static uint32_t GetEchoCount() { return EchoCount(); }
        static void     IncrementEchoCount() { EchoCount()++; }

        // Records a senderId→msgId pair in the routing map for deduplication.
        // Call after MessageExists() to mark this message as seen.
        static void RecordRouting(uint32_t senderId, uint32_t msgId)
        {
            if (xSemaphoreTake(MessageAccessMutex(), portMAX_DELAY) == pdTRUE)
            {
                RoutingMap()[senderId] = msgId;
                xSemaphoreGive(MessageAccessMutex());
            }
        }

        // Returns true if we have already recorded a message with this exact
        // (senderId, msgId) pair — used to compute isNew before RecordRouting.
        static bool MessageExists(uint32_t senderId, uint32_t msgId)
        {
            if (xSemaphoreTake(MessageAccessMutex(), portMAX_DELAY) == pdTRUE)
            {
                auto& m = RoutingMap();
                auto it = m.find(senderId);
                bool exists = (it != m.end() && it->second == msgId);
                xSemaphoreGive(MessageAccessMutex());
                return exists;
            }
            return false;
        }

        static bool RegisterMessageType(uint32_t schemaGuid, MessageCreator creator)
        {
            auto& creators = Creators();
            if (creators.find(schemaGuid) != creators.end()) { return false; }
            creators[schemaGuid] = creator;
            return true;
        }

        static std::shared_ptr<LoraMessageInterface> DeserializeMessage(const uint8_t* buffer, size_t len)
        {
            StaticJsonDocument<MSG_BASE_SIZE> doc;
            StaticJsonDocument<MSG_BASE_SIZE> payloadDoc;
            JsonObject payload;

            // Single msgpack document. The routing header is always plaintext; the "p" field is
            // either a map (plaintext payload) or a str of raw AES-CBC ciphertext (encrypted).
            // The IV lives in the header as the "v" str.
            if (deserializeMsgPack(doc, buffer, len) != DeserializationError::Ok)
            {
                return nullptr;
            }

            JsonVariant payloadVar = doc[LoraMessageInterface::KEY_PAYLOAD];

            if (payloadVar.is<JsonObject>())
            {
                // Plaintext payload. If we have a key set this message belongs to a different
                // "chatroom" (or is unencrypted noise) — never dispatch plaintext when encrypting.
                // LoraManager still routes it via ReadBaseFields + RelayMessage.
                if (EncryptionEnabled()) { return nullptr; }
                payload = payloadVar.as<JsonObject>();
            }
            else
            {
                // Encrypted payload. We can only read it with a matching key; a different
                // chatroom's key fails the PKCS7 check in Decrypt and returns nullptr.
                if (!EncryptionEnabled()) { return nullptr; }

                JsonString cipher = payloadVar.as<JsonString>();
                if (cipher.isNull()) { return nullptr; }

                uint8_t iv[EncryptionUtils::IV_SIZE]{};
                JsonString ivStr = doc[LoraMessageInterface::KEY_IV].as<JsonString>();
                if (ivStr.isNull() || ivStr.size() != EncryptionUtils::IV_SIZE) { return nullptr; }
                memcpy(iv, ivStr.c_str(), EncryptionUtils::IV_SIZE);

                uint8_t plaintext[MSG_BASE_SIZE];
                size_t plaintextLen = 0;
                if (!EncryptionUtils::Decrypt(reinterpret_cast<const uint8_t*>(cipher.c_str()),
                                              cipher.size(), plaintext, plaintextLen,
                                              EncryptionKey(), iv))
                {
                    return nullptr;
                }

                if (deserializeMsgPack(payloadDoc, plaintext, plaintextLen) != DeserializationError::Ok)
                {
                    return nullptr;
                }
                payload = payloadDoc.as<JsonObject>();
            }

            if (payload.isNull()) { return nullptr; }

            uint32_t guid = computeSchemaGuid(payload);
            auto& creators = Creators();
            auto it = creators.find(guid);
            if (it == creators.end()) { return nullptr; }

            auto msg = it->second(payload);
            if (!msg) { return nullptr; }

            msg->deserialize(doc);
            if (!msg->IsValid()) { return nullptr; }

            return msg;
        }

        static bool SerializeMessage(const std::shared_ptr<LoraMessageInterface>& msg,
                                     uint8_t* buffer, size_t& outLen)
        {
            if (EncryptionEnabled())
            {
                ESP_LOGI("LoraUtilities", "Serializing encrypted message");
                // Generate a fresh random IV for this message and store it on the object
                // so it is serialized into the base header along with the other fields.
                EncryptionUtils::GenerateIV(msg->iv);
            }
            else
            {
                memset(msg->iv, 0, EncryptionUtils::IV_SIZE);
            }

            StaticJsonDocument<MSG_BASE_SIZE> doc;
            if (!msg->serialize(doc)) 
            { 
                ESP_LOGE("LoraUtilities", "Failed to serialize message");
                return false; 
            }
            else
            {
                std::string debugStr;
                serializeJson(doc, debugStr);
            }

            if (EncryptionEnabled())
            {
                // Serialize the payload object on its own and encrypt it, then swap the payload
                // map in the document for the raw ciphertext as a msgpack str. The result is a
                // single self-describing document — no 0xEE framing — where "p" being a str
                // (rather than a map) signals encryption.
                JsonObject payloadObj = doc[LoraMessageInterface::KEY_PAYLOAD].as<JsonObject>();

                uint8_t plaintext[MSG_BASE_SIZE];
                size_t plaintextLen = serializeMsgPack(payloadObj, plaintext, sizeof(plaintext));

                uint8_t ciphertext[MSG_BASE_SIZE + 16];
                size_t ciphertextLen = 0;
                if (!EncryptionUtils::Encrypt(plaintext, plaintextLen, ciphertext, ciphertextLen,
                                              EncryptionKey(), msg->iv))
                {
                    return false;
                }

                doc.remove(LoraMessageInterface::KEY_PAYLOAD);
                doc[LoraMessageInterface::KEY_PAYLOAD] =
                    JsonString(reinterpret_cast<const char*>(ciphertext), ciphertextLen, JsonString::Copied);
            }

            outLen = serializeMsgPack(doc, buffer, MSG_BASE_SIZE);
            return outLen > 0;
        }

        static bool MessagePackSanityCheck(JsonDocument& doc)
        {
            uint8_t buffer[MSG_BASE_SIZE];
            size_t len = serializeMsgPack(doc, buffer, sizeof(buffer));
            StaticJsonDocument<MSG_BASE_SIZE> doc2;
            auto rc = deserializeMsgPack(doc2, buffer, len);
            if (rc != DeserializationError::Ok)
            {
                ESP_LOGE(TAG, "MessagePackSanityCheck failed: %s", rc.c_str());
                return false;
            }
            return true;
        }

        // Per-type event map — keyed by schema GUID.
        // Application registers handlers via MessageTypeReceived(GUID) += ...
        static std::unordered_map<uint32_t, EventHandler<std::shared_ptr<LoraMessageInterface>, bool>>& MessageEvents()
        {
            static std::unordered_map<uint32_t, EventHandler<std::shared_ptr<LoraMessageInterface>, bool>> events;
            return events;
        }

        static EventHandler<std::shared_ptr<LoraMessageInterface>, bool>& MessageTypeReceived(uint32_t schemaGuid)
        {
            return MessageEvents()[schemaGuid];
        }

        // Fired when SetMyLastBroadcast is called (i.e. the user sends a new message).
        // Subscribe to reset application-layer state (e.g. echo counts).
        static EventHandler<>& MyLastBroadcastChanged()
        {
            static EventHandler<> e;
            return e;
        }

        static bool MyLastBroadcastExists() { return MyLastBroadcastMsg() != nullptr; }

        // Getters / setters via Meyers singletons
        static int& MessageSendQueueID()
        {
            static int id = -1;
            return id;
        }

        static std::string& UserName()
        {
            static std::string name = "User";
            return name;
        }

        static uint8_t& NodeID()
        {
            static uint8_t id = 0;
            return id;
        }

        static uint8_t& DefaultSendAttempts()
        {
            static uint8_t n = LORA_DEFAULT_SEND_ATTEMPTS;
            return n;
        }

        static constexpr const char* SETTING_LORA_PASSWORD = "Channel Key";
        static constexpr size_t      LORA_PASSWORD_MAX_LEN = 21;

        static void GenerateDefaultSettings(std::vector<std::shared_ptr<FilesystemModule::SettingsInterface>>& settings)
        {
            auto pw = std::make_shared<FilesystemModule::StringSetting>(
                SETTING_LORA_PASSWORD, "", LORA_PASSWORD_MAX_LEN);
            settings.push_back(pw);

            auto frequency = std::make_shared<FilesystemModule::FloatSetting>("Frequency", 914.9, 902.3, 914.9, 0.2);
            settings.push_back(frequency);

            auto broadcastAttempts = std::make_shared<FilesystemModule::IntSetting>("Num Broadcasts", 3, 1, 5, 1);
            settings.push_back(broadcastAttempts);
        }

        static void UpdateSettings(JsonDocument& settings)
        {
            if (settings.containsKey(SETTING_LORA_PASSWORD))
            {
                const std::string pw = settings[SETTING_LORA_PASSWORD].as<std::string>();
                EncryptionEnabled() = !pw.empty();
                if (EncryptionEnabled())
                {
                    // Empty password → plaintext mode; DeriveKey must never be called with "".
                    EncryptionUtils::DeriveKey(pw, EncryptionKey());
                }
            }

            UserName() = settings["User Name"].as<std::string>();
            DefaultSendAttempts() = settings["Num Broadcasts"] | (uint8_t)3;
        }

        // Reads sender / msgID / bouncesLeft from ANY message format (encrypted or plaintext)
        // without attempting decryption. Used by LoraManager to make routing decisions for
        // messages that cannot be dispatched (wrong chatroom).
        static bool ReadBaseFields(const uint8_t* buffer, size_t len,
                                   uint32_t& sender, uint32_t& msgID, uint8_t& bouncesLeft)
        {
            // The routing header is always plaintext top-level msgpack, regardless of whether
            // the "p" payload is an encrypted str — so a single deserialize covers both formats.
            StaticJsonDocument<MSG_BASE_SIZE> doc;
            if (deserializeMsgPack(doc, buffer, len) != DeserializationError::Ok) 
            { 
                ESP_LOGE("LoraUtilities", "Failed to deserialize message");
                return false; 
            }
            else
            {
                std::string debugStr;
                serializeJson(doc, debugStr);
            }
            sender      = doc[LoraMessageInterface::KEY_FROM]         | 0u;
            msgID       = doc[LoraMessageInterface::KEY_MSG_ID]       | 0u;
            bouncesLeft = doc[LoraMessageInterface::KEY_BOUNCES_LEFT] | uint8_t(0);
            return sender != 0 && msgID != 0;
        }

        // Re-packs a received message with a decremented bouncesLeft for raw relay.
        // For encrypted messages the ciphertext is copied verbatim so downstream devices
        // with the correct key can still decrypt. Used when DeserializeMessage returns nullptr
        // (wrong chatroom) but the message should still be forwarded.
        static bool RelayMessage(const uint8_t* buffer, size_t len,
                                 uint8_t* outBuffer, size_t& outLen, uint8_t newBouncesLeft)
        {
            // One document round-trips both formats: the encrypted "p" and "v" strs (and the
            // plaintext "p" map) re-serialize byte-identical, so the ciphertext is preserved
            // for downstream nodes without any special-casing — only bouncesLeft changes.
            StaticJsonDocument<MSG_BASE_SIZE> doc;
            if (deserializeMsgPack(doc, buffer, len) != DeserializationError::Ok) 
            { 
                ESP_LOGW("LoraUtilities", "Failed to deserialize message for relay");
                return false; 
            }
            else
            {
                std::string debugStr;
                serializeMsgPack(doc, debugStr);
            }
            doc[LoraMessageInterface::KEY_BOUNCES_LEFT] = newBouncesLeft;
            outLen = serializeMsgPack(doc, outBuffer, MSG_BASE_SIZE);
            return outLen > 0;
        }

    private:
        static bool& EncryptionEnabled()
        {
            static bool e = false;
            return e;
        }

        static uint8_t* EncryptionKey()
        {
            static uint8_t k[EncryptionUtils::KEY_SIZE]{};
            return k;
        }

        // senderId → most-recently-seen msgId, for MessageExists() deduplication.
        static std::map<uint32_t, uint32_t>& RoutingMap()
        {
            static std::map<uint32_t, uint32_t> m;
            return m;
        }

        static std::shared_ptr<LoraMessageInterface>& MyLastBroadcastMsg()
        {
            static std::shared_ptr<LoraMessageInterface> msg;
            return msg;
        }

        static uint32_t& EchoCount()
        {
            static uint32_t n = 0;
            return n;
        }

        static std::unordered_map<uint32_t, MessageCreator>& Creators()
        {
            static std::unordered_map<uint32_t, MessageCreator> c;
            return c;
        }

        static SemaphoreHandle_t& MessageAccessMutex()
        {
            static StaticSemaphore_t buf;
            static SemaphoreHandle_t h = xSemaphoreCreateMutexStatic(&buf);
            return h;
        }
    };
}
