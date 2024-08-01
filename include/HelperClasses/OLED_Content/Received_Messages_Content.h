#pragma once

#include "OLED_Content.h"
#include "Network_Manager.h"
#include "MessagePing.h"
#include "Navigation_Manager.h"
#include "Settings_Manager.h"
#include "LED_Manager.h"
#include <vector>

class Received_Messages_Content : public OLED_Content
{
public:
    Received_Messages_Content(Adafruit_SSD1306 *disp, bool showReadMsgs = false, bool wrapAround = true);
    ~Received_Messages_Content();

    void printContent();
    void encUp();
    void encDown();

    void updateMessages() { printContent(); }

    MessageBase *getCurrentMessage()
    {
        if (Network_Manager::messages.size() == 0)
        {
            return nullptr;
        }
        return Network_Manager::findMessageByIdx(msgIdx);
    }

    uint16_t getMsgIdx() { return msgIdx; }
    bool getShowReadMsgs() { return showReadMsgs; }
    bool getWrapAround() { return wrapAround; }
    size_t getSelectedIndex() { return msgIdx; }
    void markMessageAsRead()
    {
        if (msgIdx < msgSenderUserIDs.size())
        {
            return;
        }
        Network_Manager::markMessageAsRead(msgSenderUserIDs[msgIdx]);
    }

private:
    void printMessageAge(uint64_t timeDiff);
    void getMessages();
    void checkAndRefreshMessages()
    {
        if (showReadMsgs)
        {
            if (Network_Manager::isLocalMsgMapOutdated(lastRefreshTime))
            {
                getMessages();
                lastRefreshTime = Network_Manager::getLastMessageInsertDeleteTime();
            }
        }
        else
        {
            if (Network_Manager::isLocalUnreadMsgMapOutdated(lastRefreshTime))
            {
                getMessages();
                lastRefreshTime = Network_Manager::getLastUnreadMessageInsertDeleteTime();
            }
        }
    }

    std::vector<uint64_t> msgSenderUserIDs;
    int64_t lastRefreshTime;
    uint16_t msgIdx = 0;
    bool showReadMsgs;
    bool wrapAround;
};