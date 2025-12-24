#include "LoRa_Test_Content.h"

LoRa_Test_Content::LoRa_Test_Content(Adafruit_SSD1306 *disp)
{
    display = disp;
    type = ContentType::LORA_TEST;
    LoraUtils::ResetMessageIterator();
}

LoRa_Test_Content::~LoRa_Test_Content()
{
}

void LoRa_Test_Content::encDown()
{
    if (LoraUtils::GetNumMessages() == 0)
    {
        ESP_LOGV(TAG, "No messages");
        return;
    }
    LoraUtils::DecrementMessageIterator();

    printContent();
}

void LoRa_Test_Content::encUp()
{
    if (LoraUtils::GetNumMessages() == 0)
    {
        ESP_LOGV(TAG, "No messages");
        return;
    }

    LoraUtils::IncrementMessageIterator();

    printContent();
}

void LoRa_Test_Content::printContent()
{
    display->fillRect(0, 8, OLED_WIDTH, OLED_HEIGHT - 16, BLACK);

    ESP_LOGV(TAG, "LoraUtils::GetNumMessages(): %d", LoraUtils::GetNumMessages());

    if (LoraUtils::GetNumMessages() == 0)
    {
        ESP_LOGV(TAG, "printContent(): No messages");
        display->setCursor(20, 12);
        display->print("No messages");
        display->display();
        return;
    }
    ESP_LOGV(TAG, "msgIdx: %d", msgIdx);
    MessageBase *msg = LoraUtils::GetCurrentMessage();

    if (msg == nullptr)
    {
        ESP_LOGV(TAG, "printContent(): Message is null. We shouldn't be here");
    return;
    }


    ESP_LOGV(TAG, "SenderName: %s", msg->senderName);
    ESP_LOGV(TAG, "MsgID: 0x%lX", (unsigned long)msg->msgID);

    display->setCursor(0, 8);
    display->print("Name: ");
    display->print(msg->senderName);

    display->setCursor(0, 16);
    display->print("MsgID: 0x");
    display->print(msg->msgID, HEX);

    display->display();

    delete msg;
}

uint8_t LoRa_Test_Content::sendBroadcast()
{
    uint32_t time = NavigationUtils::GetTime().value();
    uint32_t date = NavigationUtils::GetDate().value();
    const char *senderName = Settings_Manager::settings["User"]["Name"]["cfgVal"].as<const char *>();
    MessageBase msg = MessageBase(time, date, 0, LoraUtils::UserID(), senderName, esp_random());

    return LoraUtils::SendMessage(&msg, 1);
}

void LoRa_Test_Content::updateMessages()
{
    ESP_LOGV(TAG, "Updating messages");
    printContent();
}

// MessageBase *LoRa_Test_Content::getCurrentMessage()
// {
//     return Network_Manager::findMessageByIdx(msgIdx);
// }