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

            if (msg->sender == UserID())
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

            // Encrypted wire format: 0xEE | uint16_t LE baseLen | base bytes (includes IV) | uint16_t LE cipherLen | cipher bytes
            if (len > 0 && buffer[0] == 0xEE)
            {
                // No early-return for !EncryptionEnabled() — if we have no key, Decrypt will fail
                // and we return nullptr. The caller (LoraManager) handles routing separately.
                if (len < 5) { return nullptr; }
                uint16_t baseLen = static_cast<uint16_t>(buffer[1]) | (static_cast<uint16_t>(buffer[2]) << 8);
                size_t baseOffset = 3;
                if (baseOffset + baseLen + 2 > len) { return nullptr; }

                if (deserializeMsgPack(doc, buffer + baseOffset, baseLen) != DeserializationError::Ok)
                {
                    return nullptr;
                }

                // Read the per-message IV from the base header before decrypting.
                uint8_t iv[EncryptionUtils::IV_SIZE]{};
                JsonArray ivArr = doc[LoraMessageInterface::KEY_IV].as<JsonArray>();
                if (!ivArr.isNull()) {
                    size_t i = 0;
                    for (uint8_t b : ivArr) { if (i < EncryptionUtils::IV_SIZE) iv[i++] = b; }
                }

                size_t cipherOffset = baseOffset + baseLen;
                uint16_t cipherLen = static_cast<uint16_t>(buffer[cipherOffset])
                                   | (static_cast<uint16_t>(buffer[cipherOffset + 1]) << 8);
                cipherOffset += 2;
                if (cipherOffset + cipherLen > len) { return nullptr; }

                uint8_t plaintext[MSG_BASE_SIZE];
                size_t plaintextLen = 0;
                if (!EncryptionUtils::Decrypt(buffer + cipherOffset, cipherLen, plaintext, plaintextLen,
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
            else
            {
                // Plaintext message. If we have encryption enabled this message is from a different
                // "chatroom" — we cannot dispatch it. Return nullptr; LoraManager routes it via
                // ReadBaseFields + RelayMessage regardless.
                if (EncryptionEnabled()) { return nullptr; }

                if (deserializeMsgPack(doc, buffer, len) != DeserializationError::Ok)
                {
                    return nullptr;
                }
                payload = doc[LoraMessageInterface::KEY_PAYLOAD].as<JsonObject>();
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
                // Generate a fresh random IV for this message and store it on the object
                // so it is serialized into the base header along with the other fields.
                EncryptionUtils::GenerateIV(msg->iv);
            }
            else
            {
                memset(msg->iv, 0, EncryptionUtils::IV_SIZE);
            }

            StaticJsonDocument<MSG_BASE_SIZE> doc;
            if (!msg->serialize(doc)) { return false; }

            if (EncryptionEnabled())
            {
                // Serialize payload separately, encrypt it with the per-message IV, then write
                // custom wire format: 0xEE | uint16_t LE baseLen | base bytes | uint16_t LE cipherLen | cipher bytes
                // The IV is embedded inside the base bytes as field KEY_IV.
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
                uint8_t baseBuffer[MSG_BASE_SIZE];
                size_t baseLen = serializeMsgPack(doc, baseBuffer, sizeof(baseBuffer));

                if (baseLen > 0xFFFF || ciphertextLen > 0xFFFF) { return false; }
                size_t totalLen = 1 + 2 + baseLen + 2 + ciphertextLen;
                if (totalLen > MSG_BASE_SIZE) { return false; }

                buffer[0] = 0xEE;
                buffer[1] = static_cast<uint8_t>(baseLen & 0xFF);
                buffer[2] = static_cast<uint8_t>((baseLen >> 8) & 0xFF);
                memcpy(buffer + 3, baseBuffer, baseLen);
                size_t cipherOffset = 3 + baseLen;
                buffer[cipherOffset]     = static_cast<uint8_t>(ciphertextLen & 0xFF);
                buffer[cipherOffset + 1] = static_cast<uint8_t>((ciphertextLen >> 8) & 0xFF);
                memcpy(buffer + cipherOffset + 2, ciphertext, ciphertextLen);
                outLen = totalLen;
                return true;
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

        static uint32_t& UserID()
        {
            static uint32_t id = 0;
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
            StaticJsonDocument<MSG_BASE_SIZE> doc;
            if (len > 0 && buffer[0] == 0xEE)
            {
                if (len < 5) { return false; }
                uint16_t baseLen = static_cast<uint16_t>(buffer[1]) | (static_cast<uint16_t>(buffer[2]) << 8);
                if (3 + baseLen > len) { return false; }
                if (deserializeMsgPack(doc, buffer + 3, baseLen) != DeserializationError::Ok) { return false; }
            }
            else
            {
                if (deserializeMsgPack(doc, buffer, len) != DeserializationError::Ok) { return false; }
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
            StaticJsonDocument<MSG_BASE_SIZE> doc;
            if (len > 0 && buffer[0] == 0xEE)
            {
                if (len < 5) { return false; }
                uint16_t baseLen = static_cast<uint16_t>(buffer[1]) | (static_cast<uint16_t>(buffer[2]) << 8);
                if (3 + baseLen > len) { return false; }
                if (deserializeMsgPack(doc, buffer + 3, baseLen) != DeserializationError::Ok) { return false; }

                doc[LoraMessageInterface::KEY_BOUNCES_LEFT] = newBouncesLeft;

                uint8_t newBase[MSG_BASE_SIZE];
                size_t newBaseLen = serializeMsgPack(doc, newBase, sizeof(newBase));
                if (newBaseLen == 0) { return false; }

                // Preserve the cipher section (cipherLen uint16 + cipher bytes) verbatim.
                size_t cipherSection    = 3 + baseLen;
                size_t cipherSectionLen = len - cipherSection;
                outLen = 3 + newBaseLen + cipherSectionLen;
                if (outLen > MSG_BASE_SIZE) { return false; }

                outBuffer[0] = 0xEE;
                outBuffer[1] = static_cast<uint8_t>(newBaseLen & 0xFF);
                outBuffer[2] = static_cast<uint8_t>((newBaseLen >> 8) & 0xFF);
                memcpy(outBuffer + 3, newBase, newBaseLen);
                memcpy(outBuffer + 3 + newBaseLen, buffer + cipherSection, cipherSectionLen);
                return true;
            }
            else
            {
                // Plaintext: re-serialize full doc with updated bouncesLeft.
                if (deserializeMsgPack(doc, buffer, len) != DeserializationError::Ok) { return false; }
                doc[LoraMessageInterface::KEY_BOUNCES_LEFT] = newBouncesLeft;
                outLen = serializeMsgPack(doc, outBuffer, MSG_BASE_SIZE);
                return outLen > 0;
            }
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
