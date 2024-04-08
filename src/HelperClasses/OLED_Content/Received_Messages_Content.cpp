#include "Received_Messages_Content.h"

Received_Messages_Content::Received_Messages_Content(Adafruit_SSD1306 *disp, bool showReadMsgs, bool wrapAround)
    : showReadMsgs(showReadMsgs), wrapAround(wrapAround)
{
    display = disp;
    type = ContentType::STATUS;
    msgIdx = 0;
}

Received_Messages_Content::~Received_Messages_Content()
{
}

void Received_Messages_Content::encDown()
{
    if (!wrapAround && msgIdx == msgSenderUserIDs.size() - 1)
    {
        return;
    }

    msgIdx++;
    if (msgIdx >= msgSenderUserIDs.size())
    {
        msgIdx = 0;
    }

    printContent();
}

void Received_Messages_Content::encUp()
{
    if (!wrapAround && msgIdx == 0)
    {
        return;
    }

    if (msgIdx == 0)
    {
        msgIdx = msgSenderUserIDs.size() - 1;
    }
    else
    {
        msgIdx--;
    }

    printContent();
}

void Received_Messages_Content::printContent()
{
    OLED_Content::clearContentArea();

#if DEBUG == 1
    Serial.println("Received_Messages_Content::printContent()");
#endif

    if (msgSenderUserIDs.size() == 0)
    {
        display->fillRect(0, 0, OLED_WIDTH / 2, 8, BLACK);
        display->fillRect(OLED_WIDTH / 2, OLED_HEIGHT - 8, OLED_WIDTH / 2, 8, BLACK);
#if DEBUG == 1
        Serial.println("No messages");
#endif
        display->setCursor(20, 12);
        display->print("No messages");
        display->display();
        return;
    }

    uint64_t userLookup = msgSenderUserIDs[msgIdx];

    Message_Base *msg = Network_Manager::getMessageEntry(userLookup);
    switch (msg->msgType)
    {
    case MessageType::MESSAGE_BASE:
    {
        display->setCursor(0, 8);
        display->print(msg->senderName);

        display->setCursor(0, 16);
        display->print(F("MsgID: "));
        display->print(msg->msgID, HEX);

        display->setCursor(110, 8);
        printMessageAge(Navigation_Manager::getTimeDifference(msg->time, msg->date));
        break;
    }
    case MessageType::MESSAGE_PING:
    {
        Message_Ping *pingMsg = (Message_Ping *)msg;
        uint64_t timeDiff = Navigation_Manager::getTimeDifference(pingMsg->time, pingMsg->date);

        display->setCursor(110, 8);
        // 128display->print(F("Recv: "));

        // Greater than one day
        printMessageAge(timeDiff);

        display->setCursor(0, 8);
        display->print(pingMsg->senderName);

        display->setCursor(0, 16);
        display->print(pingMsg->status);

        LED_Manager::lightRing(pingMsg->color_R, pingMsg->color_G, pingMsg->color_B);
        break;
    }
    default:
        break;
    }

    display->display();

    delete msg;
}

void Received_Messages_Content::printMessageAge(uint64_t timeDiff)
{
    if (timeDiff > 0xFFFFFFFF)
    {
        display->print(">1d");
    }
    else if ((timeDiff & 0xFF000000) >> 24)
    {
        uint8_t hours = (timeDiff & 0xFF000000) >> 24;
        display->print(hours);
        display->print(F("h"));
    }
    else if ((timeDiff && 0xFF0000) >> 16)
    {
        uint8_t minutes = (timeDiff && 0xFF0000) >> 16;
        display->print(minutes);
        display->print(F("m"));
    }
    else
    {
        display->print(F("<1m"));
    }
}

void Received_Messages_Content::getMessages()
{
    msgSenderUserIDs.clear();
    DynamicJsonDocument *doc;
    uint64_t currentUserIDPointedTo = 0;

    if (msgIdx < msgSenderUserIDs.size())
    {
        currentUserIDPointedTo = msgSenderUserIDs[msgIdx];
    }

    if (showReadMsgs)
    {
        if (lastRefreshTime < Network_Manager::getLastMessageInsertDeleteTime())
        {
            lastRefreshTime = Network_Manager::getLastMessageInsertDeleteTime();
            doc = Network_Manager::getMessages();
        }
        else
        {
            return;
        }
    }
    else
    {
        if (lastRefreshTime < Network_Manager::getLastUnreadMessageInsertDeleteTime())
        {
            lastRefreshTime = Network_Manager::getLastUnreadMessageInsertDeleteTime();
            doc = Network_Manager::getMessagesUnreadMessages();
        }
        else
        {
            return;
        }
    }

    for (uint16_t i = 0; i < (*doc)["UserIDs"].size(); i++)
    {
        msgSenderUserIDs.push_back((*doc)[i]);
    }

    auto it = std::find(msgSenderUserIDs.begin(), msgSenderUserIDs.end(), currentUserIDPointedTo);
    if (it != msgSenderUserIDs.end())
    {
        msgIdx = std::distance(msgSenderUserIDs.begin(), it);
    }
    else
    {
        msgIdx = 0;
    }

    delete doc;
}
