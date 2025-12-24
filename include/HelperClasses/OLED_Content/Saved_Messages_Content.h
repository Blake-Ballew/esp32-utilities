#pragma once

#include "OLED_Content.h"
#include "Settings_Manager.h"
#include "LED_Manager.h"
#include <vector>


struct Saved_Message
{
    int idx;
    const char *msg;

    Saved_Message()
    {
        idx = -1;
        msg = nullptr;
    }

    Saved_Message(int idx, const char *msg)
    {
        this->idx = idx;
        this->msg = msg;
    }

    Saved_Message(const Saved_Message &msg)
    {
        idx = msg.idx;
        this->msg = msg.msg;
    }

    ~Saved_Message()
    {
    }
};

namespace {
    uint8_t promptSelectMode = 0;
    uint8_t promptNumSavedMsges = 1;
}

class Saved_Messages_Content : public OLED_Content
{
public:
    Saved_Messages_Content()
    {
        type = ContentType::SAVED_MSG;
        msgIdx = 0;
        msgList.clear();

        loadMessagesFromEEPROM();
    }

    ~Saved_Messages_Content() {}

    void printContent()
    {
        if (Settings_Manager::savedMessages["Messages"].as<JsonArray>().size() == 0)
        {
            ESP_LOGD(TAG, "No saved messages");
            display->display();
            return;
        }

        // Clear content area
        display->fillRect(0, 8, OLED_WIDTH, OLED_HEIGHT - 16, BLACK);

        // Print message
        if (promptMode == promptNumSavedMsges) {
            display->setCursor(Display_Utils::centerTextHorizontal(18), Display_Utils::selectTextLine(2));
            display->printf("Total saved %d/%d", msgList.size(), Settings_Manager::maxMsges);   
        }
        else if (promptMode == promptSelectMode) {
            display->setCursor(Display_Utils::centerTextHorizontal("Select a message"), Display_Utils::selectTextLine(2));
            display->print("Select a message");
        }
        
        display->setCursor(Display_Utils::centerTextHorizontal(msgList[msgIdx].msg), Display_Utils::selectTextLine(3));
        display->print(msgList[msgIdx].msg);

        LED_Manager::displayScrollWheel(msgIdx, msgList.size());

        display->display();
    }

    void encUp()
    {
        if (msgList.size() == 0)
        {
            return;
        }

        if (msgIdx == 0)
        {
            msgIdx = msgList.size() - 1;
        }
        else
        {
            msgIdx--;
        }

        printContent();
    }

    void encDown()
    {
        if (msgList.size() == 0)
        {
            return;
        }

        msgIdx++;
        if (msgIdx >= msgList.size())
        {
            msgIdx = 0;
        }

        printContent();
    }

    void deleteMsg()
    {
        if (msgList[msgIdx].idx == -1)
        {
            return;
        }

        Settings_Manager::deleteMessage(msgList[msgIdx].idx);
        msgList.erase(msgList.begin() + msgIdx);
        if (msgIdx >= msgList.size())
        {
            msgIdx = msgList.size() - 1;
        }
        printContent();
    }

    void saveMsg(const char *msg, uint8_t msgLength)
    {
        Settings_Manager::addMessage(msg, msgLength);
        loadMessagesFromEEPROM();
    }

    void insertMessage(const char *msg)
    {
        Saved_Message newMsg;
        newMsg.idx = -1;
        newMsg.msg = msg;
        additionalMsgList.insert(additionalMsgList.begin(), newMsg);
    }

    void loadMessages()
    {
        ESP_LOGD(TAG, "Loading messages");

        msgList.clear();

        ESP_LOGV(TAG, "Copying messages from additional message list");
        if (additionalMsgList.size() > 0)
        {
            ESP_LOGV(TAG, "additionalMsgList has %zu elements", additionalMsgList.size());
            for (const Saved_Message &it : additionalMsgList)
            {
                msgList.push_back(it);
            }
        }
        else
        {
            ESP_LOGV(TAG, "No additional messages");
        }

        ESP_LOGV(TAG, "Copying messages from EEPROM list");
        if (eepromMsgList.size() > 0)
        {
            for (const Saved_Message &it : eepromMsgList)
            {
                msgList.push_back(it);
            }
        }

        ESP_LOGD(TAG, "Loaded %zu messages", msgList.size());
    }

    const char *getCurrentMessage()
    {
        return Settings_Manager::savedMessages["Messages"][msgIdx].as<const char *>();
    }

    void clearAdditionalMessages()
    {
        additionalMsgList.clear();
    }

    void setSelectMsgPrompt()
    {
        promptMode = promptSelectMode;
    }

    void setNumSavedMsgPrompt()
    {
        promptMode = promptNumSavedMsges;
    }

protected:
    void loadMessagesFromEEPROM()
    {
        ESP_LOGD(TAG, "Loading messages from eeprom");

        eepromMsgList.clear();
        size_t idx = 0;

        for (auto it : Settings_Manager::savedMessages["Messages"].as<JsonArray>())
        {
            Saved_Message newMsg;
            newMsg.idx = idx;
            newMsg.msg = it.as<const char *>();
            eepromMsgList.push_back(newMsg);
            idx++;
        }

        ESP_LOGD(TAG, "Loaded %zu messages", eepromMsgList.size());
    }

    uint8_t promptMode;
    size_t msgIdx;
    std::vector<Saved_Message> msgList;
    std::vector<Saved_Message> eepromMsgList;
    std::vector<Saved_Message> additionalMsgList;
};