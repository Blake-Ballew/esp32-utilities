#include "Saved_Msg_Content.h"

Saved_Msg_Content::Saved_Msg_Content(Adafruit_SSD1306 *disp)
{
    display = disp;
    type = ContentType::SAVED_MSG;
    msgIdx = 0;
    msgListSize = Settings_Manager::getNumMsges();
    printContent();
}

Saved_Msg_Content::~Saved_Msg_Content()
{
}

void Saved_Msg_Content::printContent()
{
    if (Settings_Manager::savedMessages["Messages"].as<JsonArray>().size() == 0)
    {
#if DEBUG == 1
        Serial.println("No saved messages");
#endif
        display->display();
        return;
    }

    // Clear content area
    display->fillRect(0, 8, OLED_WIDTH, OLED_HEIGHT - 16, BLACK);

    // Print message
    display->setCursor(OLED_Content::centerTextHorizontal(Settings_Manager::savedMessages["Messages"][msgIdx].as<const char *>()), OLED_Content::centerTextVertical());
    display->print(Settings_Manager::savedMessages["Messages"][msgIdx].as<const char *>());

    LED_Manager::displayScrollWheel(msgIdx, msgListSize);

    display->display();
}

void Saved_Msg_Content::encUp()
{
    if (msgListSize == 0)
    {
        return;
    }

    if (msgIdx == 0)
    {
        msgIdx = msgListSize - 1;
    }
    else
    {
        msgIdx--;
    }

    printContent();
}

void Saved_Msg_Content::encDown()
{
    if (msgListSize == 0)
    {
        return;
    }

    msgIdx++;
    if (msgIdx >= msgListSize)
    {
        msgIdx = 0;
    }

    printContent();
}

void Saved_Msg_Content::deleteMsg()
{
    Settings_Manager::deleteMessage(msgIdx);
    msgListSize = Settings_Manager::getNumMsges();
    if (msgIdx >= msgListSize)
    {
        msgIdx = msgListSize - 1;
    }
    printContent();
}

void Saved_Msg_Content::saveMsg(const char *msg, uint8_t msgLength)
{
    Settings_Manager::addMessage(msg, msgLength);
    msgListSize = Settings_Manager::getNumMsges();
    msgIdx = msgListSize - 1;
    printContent();
}