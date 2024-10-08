#include "Home_Content.h"

Home_Content::Home_Content(Adafruit_SSD1306 *display)
{
    this->type = ContentType::HOME;
    this->display = display;
    this->contentMode = 1;
    Display_Utils::enableRefreshTimer(HOME_CONTENT_TIMER_PERIOD);
}

Home_Content::~Home_Content()
{
}

void Home_Content::printContent()
{

    NavigationUtils::UpdateGPS();
    // TinyGPSDate date;
    TinyGPSTime time = NavigationUtils::GetTime();
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

        if (FilesystemUtils::SettingsFile()["24H Time"].as<bool>())
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

    size_t unreadMsgs = LoraUtils::GetNumUnreadMessages();

    /*display->setCursor(Display_Utils::alignTextLeft(), Display_Utils::selectTextLine(3));
    display->printf("Unread: %d", unreadMsgs);*/

    OLED_Content::drawBellIcon(Display_Utils::alignTextLeft(3), Display_Utils::selectTextLine(2), System_Utils::silentMode);

    OLED_Content::drawMessageIcon(Display_Utils::alignTextLeft(6), Display_Utils::selectTextLine(2));
    display->setCursor(Display_Utils::alignTextLeft(8), Display_Utils::selectTextLine(2));
    display->printf(":%d", unreadMsgs);

    if (unreadMsgs > 0)
    {
        TextFormat format;
        format.horizontalAlignment = ALIGN_CENTER_HORIZONTAL;
        format.verticalAlignment = TEXT_LINE;
        auto textLine = Display_Utils::SelectBottomTextLine();
        format.line = textLine;

        Display_Utils::printFormattedText("v", format);
        format.line--;

        Display_Utils::printFormattedText("Msgs", format);
    }

    if (LoraUtils::MyLastBroacastExists())
    {
        TextFormat format;
        format.horizontalAlignment = ALIGN_CENTER_HORIZONTAL;
        format.verticalAlignment = TEXT_LINE;
        format.line = 1;

        Display_Utils::printFormattedText("^", format);
    }

    display->display();
}

void Home_Content::stop()
{
#if DEBUG == 1
    Serial.println("Stopping Home Content");
#endif
    Display_Utils::disableRefreshTimer();
}

void Home_Content::start()
{
#if DEBUG == 1
    Serial.println("Starting Home Content");
#endif
    Display_Utils::enableRefreshTimer(HOME_CONTENT_TIMER_PERIOD);
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
