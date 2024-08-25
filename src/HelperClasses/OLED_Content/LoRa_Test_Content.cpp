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
#if DEBUG == 1
        Serial.println("No messages");
#endif
        return;
    }
    LoraUtils::DecrementMessageIterator();
    
    printContent();
}

void LoRa_Test_Content::encUp()
{
    if (LoraUtils::GetNumMessages() == 0)
    {
#if DEBUG == 1
        Serial.println("No messages");
#endif
        return;
    }

    LoraUtils::IncrementMessageIterator();

    printContent();
}

void LoRa_Test_Content::printContent()
{
    display->fillRect(0, 8, OLED_WIDTH, OLED_HEIGHT - 16, BLACK);

#if DEBUG == 1
    Serial.print("LoraUtils::GetNumMessages(): ");
    Serial.println(LoraUtils::GetNumMessages());
#endif

    if (LoraUtils::GetNumMessages() == 0)
    {
#if DEBUG == 1
        Serial.println("printContent(): No messages");
#endif
        display->setCursor(20, 12);
        display->print("No messages");
        display->display();
        return;
    }
#if DEBUG == 1
    Serial.print("msgIdx: ");
    Serial.println(msgIdx);
#endif
    MessageBase *msg = LoraUtils::GetCurrentMessage();

    if (msg == nullptr)
    {
#if DEBUG == 1
        Serial.println("printContent(): Message is null. We shouldn't be here");
#endif
    return;
    }


#if DEBUG == 1
    Serial.print("SenderName: ");
    Serial.println(msg->senderName);
    Serial.print("MsgID: 0x");
    Serial.println(msg->msgID, HEX);
#endif

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
#if DEBUG == 1
    Serial.println("Updating messages");
#endif
    printContent();
}

// MessageBase *LoRa_Test_Content::getCurrentMessage()
// {
//     return Network_Manager::findMessageByIdx(msgIdx);
// }