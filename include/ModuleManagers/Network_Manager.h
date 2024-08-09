// #pragma once

// #include <MessagePing.h>
// // #include <UserID.h>
// #include <Settings_Manager.h>
// #include <globalDefines.h>
// #include <RH_RF95.h>
// #include <RHMesh.h>
// #include <RHSoftwareSPI.h>
// #include <RHHardwareSPI.h>
// #include <Map>
// #include <ArduinoJson.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "EventDeclarations.h"
// #include "Display_Utils.h"

// #define RH_DRIVER RH_RF95
// #define RFM95_CS 15
// #define RFM95_Int 18
// #define RF95_FREQ 915.0
// #define RF95_MODEM_CONFIG RH_RF95::Bw125Cr45Sf128
// #define RF95_TX_PWR 23

// #define RF95_SPI_SCK 14
// #define RF95_SPI_MISO 12
// #define RF95_SPI_MOSI 13

// #define DEFAULT_NODE_ID 1

// #define MESSAGE_QUEUE_MAX 8
// #define RETURN_CODE_UNABLE_TO_QUEUE 255
// #define MESSAGE_QUEUE_TIMEOUT 15000

// namespace
// {
//     const char *MESSAGE_SENT PROGMEM = "Message sent";
//     const char *NO_ROUTE PROGMEM = "No route";
//     const char *DELIVERY_FAILED PROGMEM = "Delivery failed";
//     const char *UNABLE_TO_QUEUE PROGMEM = "Unable to queue";
//     const char *UNKNOWN_ERROR PROGMEM = "Unknown error";
// }

// struct MessageQueueItem
// {
//     MessageBase *msg;
//     bool isBroadcast;
//     uint64_t user;
//     uint8_t *returnCode;
// };

// class Network_Manager
// {
// public:
//     static RHReliableDatagram *manager;
//     static RH_DRIVER driver;
//     static RHHardwareSPI rf_spi;

//     static std::map<uint64_t, MessageBase *> messages;
//     static std::map<uint64_t, MessageBase *> messagesSent;

//     static QueueHandle_t messageQueue;
//     static StaticQueue_t messageQueueBuffer;

//     static SemaphoreHandle_t messageAccessSemaphore;
//     static StaticSemaphore_t messageAccessSemaphoreBuffer;

//     static MessageBase *lastBroadcast;

//     static uint8_t buffer[RH_MESH_MAX_MESSAGE_LEN];

//     // Implement later. Software defined channel hopping. Only store mesh nodes on the same channel
//     // static uint8_t channel;

//     // Essentially an IP address. Will be used to determine the node type
//     static uint8_t nodeID;

//     // User ID. Will eventually be used as a public key
//     static uint64_t userID;

//     static bool init();

//     static uint8_t sendBroadcastMessage(MessageBase *msg);
//     static uint8_t sendMessageToUser(uint64_t user, MessageBase *msg);
//     static void rebroadcastMessage(MessageBase *msg);

//     static uint8_t queueMessageToUser(uint64_t user, MessageBase *msg);
//     static uint8_t queueBroadcastMessage(MessageBase *msg);
//     static uint8_t queueMessage(MessageBase *msg);

//     static void listenForMessages(void *taskParams);

//     static uint8_t findFreeNodeID();
//     static MessageBase *findMessageByIdx(uint16_t idx);

//     static ArduinoJson::JsonArray getStatusList();

//     // static std::map<uint64_t, Message_Base *>::iterator getUnreadBegin();
//     // static std::map<uint64_t, Message_Base *>::iterator getUnreadEnd();
//     // static std::map<uint64_t, Message_Base *>::iterator incrementUnreadIterator(std::map<uint64_t, Message_Base *>::iterator it);
//     // static std::map<uint64_t, Message_Base *>::iterator decrementUnreadIterator(std::map<uint64_t, Message_Base *>::iterator it);
//     // static std::map<uint64_t, Message_Base *>::iterator getBeginIterator();
//     // static std::map<uint64_t, Message_Base *>::iterator getEndIterator();

//     static void loadStatusList();

//     // Message Delete Entry
//     static void deleteMessageEntry(uint64_t user);

//     // Message Create/Update Entry
//     static void createUpdateMessageEntry(uint64_t user, MessageBase *msg);
//     static void markMessageAsRead(uint64_t user);

//     // Message Get Entry
//     static MessageBase *getMessageEntry(uint64_t user);
//     static MessageBase *cloneMessageEntry(uint64_t user);

//     // Get List of userIDs with messages in the respective map
//     static ArduinoJson::DynamicJsonDocument *getMessages();
//     static ArduinoJson::DynamicJsonDocument *getMessagesUnreadMessages();

//     // Message map timestamps
//     static int64_t getLastMessageInsertDeleteTime() { return lastMessageInsertDeleteTime; }
//     static int64_t getLastUnreadMessageInsertDeleteTime() { return lastUnreadMessageInsertDeleteTime; }
//     static bool isLocalMsgMapOutdated(int64_t lastRefresh) { return lastMessageInsertDeleteTime > lastRefresh; }
//     static bool isLocalUnreadMsgMapOutdated(int64_t lastRefresh) { return lastUnreadMessageInsertDeleteTime > lastRefresh; }

//     // Get map sizes
//     static size_t getNumMessages();
//     static size_t getNumUnreadMessages();

//     static const char *getReturnCodeString(uint8_t returnCode);

// private:
//     // static uint8_t findNodeIDofUser(uint64_t user);

//     static ArduinoJson::DynamicJsonDocument statusList;
//     static uint8_t messageDataBuffer[];
//     static uint8_t numRetries;
//     static int64_t lastMessageInsertDeleteTime;
//     static int64_t lastUnreadMessageInsertDeleteTime;
// };
