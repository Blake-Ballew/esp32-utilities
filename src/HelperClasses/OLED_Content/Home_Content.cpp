#include "Home_Content.h"

Home_Content *Home_Content::thisInstance = nullptr;

Home_Content::Home_Content(Adafruit_SSD1306 *display)
{
    this->type = ContentType::HOME;
    this->display = display;
    this->contentMode = 1;
    System_Utils::changeTimerPeriod(refreshTimerID, HOME_CONTENT_TIMER_PERIOD);
    System_Utils::startTimer(refreshTimerID);
}

Home_Content::~Home_Content()
{
}

void Home_Content::printContent()
{

    Navigation_Manager::updateGPS();
    Navigation_Manager::read();
    // TinyGPSDate date;
    TinyGPSTime time = Navigation_Manager::getTime();
    if (time.isValid())
    {
        // Adjust for timezone -4
        int hour = time.hour();
        if (hour < 4)
        {
            hour += 20;
        }
        else
        {
            hour -= 4;
        }

        if (Settings_Manager::settings["Device"]["24HTime"].as<bool>())
        {
            display->setCursor(Display_Utils::alignTextRight(5), Display_Utils::selectTextLine(2));
            display->printf("%02d:%02d", hour, time.minute());
        }
        else
        {
            display->setCursor(Display_Utils::alignTextRight(8), Display_Utils::selectTextLine(2));
            display->printf("%02d:%02d %s", hour % 12, time.minute(), hour < 12 ? "AM" : "PM");
        }
    }
    else
    {
        display->setCursor(Display_Utils::alignTextRight(6), Display_Utils::selectTextLine(2));
        display->printf("No GPS");
    }

    OLED_Content::drawBatteryIcon(Display_Utils::alignTextLeft(0), Display_Utils::selectTextLine(2), System_Utils::getBatteryPercentage());

    size_t unreadMsgs = Network_Manager::getNumUnreadMessages();

    /*display->setCursor(Display_Utils::alignTextLeft(), Display_Utils::selectTextLine(3));
    display->printf("Unread: %d", unreadMsgs);*/

    OLED_Content::drawBellIcon(Display_Utils::alignTextLeft(3), Display_Utils::selectTextLine(2), System_Utils::silentMode);

    OLED_Content::drawMessageIcon(Display_Utils::alignTextLeft(6), Display_Utils::selectTextLine(2));
    display->setCursor(Display_Utils::alignTextLeft(8), Display_Utils::selectTextLine(2));
    display->printf(":%d", unreadMsgs);

    if (unreadMsgs > 0)
    {
        display->drawLine(OLED_WIDTH / 2, OLED_HEIGHT - 1, (OLED_WIDTH / 2) - 3, OLED_HEIGHT - 3, WHITE);
        display->drawLine((OLED_WIDTH / 2) + 1, OLED_HEIGHT - 1, (OLED_WIDTH / 2) + 4, OLED_HEIGHT - 3, WHITE);
        display->setCursor(Display_Utils::alignTextLeft(6), Display_Utils::selectTextLine(4));
        display->print("Msgs");
    }

    display->display();
}

void Home_Content::stop()
{
#if DEBUG == 1
    Serial.println("Stopping Home Content");
#endif
    System_Utils::stopTimer(refreshTimerID);
}

void Home_Content::start()
{
#if DEBUG == 1
    Serial.println("Starting Home Content");
#endif
    System_Utils::changeTimerPeriod(OLED_Content::refreshTimerID, HOME_CONTENT_TIMER_PERIOD);
    System_Utils::startTimer(OLED_Content::refreshTimerID);
}

void Home_Content::timerCallback(TimerHandle_t xTimer)
{
    thisInstance->printContent();
}

MessageBase *Home_Content::getCurrentMessage()
{
    if (contentMode == HOME_CONTENT_MESSAGES)
    {
        return msgIterator->second;
    }
    return nullptr;
}

void Home_Content::encUp()
{
    /*     switch (contentMode)
        {
        case HOME_CONTENT_MESSAGES:
        {
            auto newIt = Network_Manager::decrementUnreadIterator(msgIterator);
            if (newIt != Network_Manager::getEndIterator())
            {
                msgIterator = newIt;
            }
            else
            {
                LED_Manager::clearRing();
                contentMode = HOME_CONTENT_MAIN;
            }
            break;
        }
        } */
}

void Home_Content::encDown()
{
    /* switch (contentMode)
    {
    case HOME_CONTENT_MAIN:
    {
        if (Network_Manager::getNumUnreadMessages() > 0)
        {
            contentMode = HOME_CONTENT_MESSAGES;
            msgIterator = Network_Manager::getUnreadBegin();
        }
        break;
    }
    case HOME_CONTENT_MESSAGES:
    {
        auto newIt = Network_Manager::incrementUnreadIterator(msgIterator);
        if (newIt != Network_Manager::getEndIterator())
        {
            msgIterator = newIt;
        }
        break;
    } */
}
