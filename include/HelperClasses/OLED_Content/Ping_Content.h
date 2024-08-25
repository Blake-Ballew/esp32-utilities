#pragma once

#include "OLED_Content.h"
#include "LoraUtils.h"
#include "NavigationUtils.h"
#include "Settings_Manager.h"
#include "MessagePing.h"

namespace
{
    const char *MESSAGE_SENT PROGMEM = "Message sent";
    const char *UNABLE_TO_SEND PROGMEM = "Unable to send";
}

class Ping_Content : public OLED_Content
{
public:
    Ping_Content(Adafruit_SSD1306 *disp, MessageBase *msg)
    {
        display = disp;
        type = ContentType::PING;
        this->msg = msg;
        statusList = Settings_Manager::savedMessages["Messages"].as<JsonArray>();
        statusIdx = 0;
    }

    ~Ping_Content() {}

    void printContent()
    {
#ifdef DEBUG
        Serial.println("Ping_Content::printContent()");
#endif

        display->fillRect(0, 8, OLED_WIDTH, OLED_HEIGHT - 16, BLACK);

        display->setCursor(0, 0);
        display->print("To: ");
        if (msg == nullptr)
        {
            display->print("Broadcast");
        }
        else
        {
            display->print(msg->senderName);
        }

        if (statusList.size() == 0)
        {
            display->setCursor(0, 16);
            display->print("No statuses");
            display->display();
            return;
        }
#if DEBUG == 1
        Serial.print("Status idx: ");
        Serial.println(statusIdx);
        Serial.print("Status list size: ");
        Serial.println(statusList.size());
#endif
        const char *statusDesc = statusList[statusIdx].as<const char *>();
#if DEBUG == 1
        Serial.print("Status desc: ");
        Serial.println(statusDesc);
#endif
        display->setCursor(Display_Utils::centerTextHorizontal(statusDesc), Display_Utils::centerTextVertical());
        display->print(statusDesc);
        display->display();

        uint8_t R = Settings_Manager::settings["User"]["Theme Red"]["cfgVal"].as<uint8_t>();
        uint8_t G = Settings_Manager::settings["User"]["Theme Green"]["cfgVal"].as<uint8_t>();
        uint8_t B = Settings_Manager::settings["User"]["Theme Blue"]["cfgVal"].as<uint8_t>();

        LED_Manager::lightRing(R, G, B);
    }
    void encUp()
    {
        if (statusIdx == 0)
        {
            statusIdx = statusList.size();
        }
        statusIdx--;
        printContent();
    }

    void encDown()
    {
        statusIdx++;
        if (statusIdx >= statusList.size())
        {
            statusIdx = 0;
        }
        printContent();
    }

    void assignMsg(MessageBase *msg)
    {
        this->msg = msg;
    }

    void unassignMsg()
    {
        this->msg = nullptr;
    }

    void sendPing()
    {
        NavigationUtils::UpdateGPS();

        uint32_t time, date;
        time = NavigationUtils::GetTime().value();
        date = NavigationUtils::GetDate().value();

        uint64_t recipient = msg == nullptr ? 0 : msg->sender;
        uint64_t sender = LoraUtils::UserID();
        const char *senderName = Settings_Manager::settings["User"]["Name"]["cfgVal"].as<const char *>();

        uint32_t msgID = esp_random();

        uint8_t R = Settings_Manager::settings["User"]["Theme Red"]["cfgVal"].as<uint8_t>();
        uint8_t G = Settings_Manager::settings["User"]["Theme Green"]["cfgVal"].as<uint8_t>();
        uint8_t B = Settings_Manager::settings["User"]["Theme Blue"]["cfgVal"].as<uint8_t>();

        TinyGPSLocation coords = NavigationUtils::GetLocation();

#if USE_FAKE_GPS_COORDS == 0
        double lat = coords.lat();
        double lng = coords.lng();
#else
        double lat = FAKE_GPS_LAT;
        double lng = FAKE_GPS_LON;
#endif

        const char *status = statusList[statusIdx].as<const char *>();
        MessagePing *msgPing = new MessagePing(time, date, recipient, sender, senderName, msgID, R, G, B, lat, lng, status);

        bool isBroadcast = msg == nullptr;

        display->clearDisplay();
        display->setCursor(Display_Utils::centerTextHorizontal(10), Display_Utils::centerTextVertical());
        display->print("Sending...");
        display->display();
        LED_Manager::clearRing();

        if (isBroadcast)
        {
#ifdef DEBUG
            Serial.println("Sending broadcast");
#endif
        }
        else
        {
#ifdef DEBUG
            Serial.print("Sending direct message to: ");
            Serial.println(msg->senderName);
#endif
        }

        auto success = LoraUtils::SendMessage(msgPing, 0);

        Display_Utils::clearContentArea();

        if (success)
        {
            display->setCursor(Display_Utils::centerTextHorizontal(MESSAGE_SENT), Display_Utils::centerTextVertical());
            display->print(MESSAGE_SENT);

            LoraUtils::SetMyLastBroadcast(msgPing);
        }
        else
        {
            display->setCursor(Display_Utils::centerTextHorizontal(UNABLE_TO_SEND), Display_Utils::centerTextVertical());
            display->print(UNABLE_TO_SEND);
        }

        delete msgPing;

        // display->fillRect(OLED_WIDTH, OLED_HEIGHT, OLED_WIDTH - 24, OLED_HEIGHT - 8, BLACK);
        display->display();
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }

    uint8_t statusIdx;

private:
    MessageBase *msg;

    JsonArray statusList;
};