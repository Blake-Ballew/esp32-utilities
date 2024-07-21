#include "Save_Confirm_Content.h"

Save_Confirm_Content::Save_Confirm_Content(Adafruit_SSD1306 *disp)
{
    display = disp;
    type = ContentType::SAVE_CONFIRM; 

    /*
    btn2text = "Save Msg";
    btn4text = "Save Coords";
    btn3text = "Back";

    btn2TextLength = strlen(btn2text);
    btn4TextLength = strlen(btn4text);
    btn3TextLength = strlen(btn3text);

    btn2CallbackID = ACTION_DEFER_CALLBACK_TO_WINDOW;
    btn4CallbackID = ACTION_DEFER_CALLBACK_TO_WINDOW;
    btn3CallbackID = ACTION_DEFER_CALLBACK_TO_WINDOW;
    */
}

Save_Confirm_Content::~Save_Confirm_Content()
{
}

void Save_Confirm_Content::printContent()
{
    Display_Utils::clearContentArea();
    display->setCursor(Display_Utils::centerTextHorizontal(5), Display_Utils::centerTextVertical());
    display->print("Save?");
    display->display();
}

void Save_Confirm_Content::encUp()
{
}

void Save_Confirm_Content::encDown()
{
}

void Save_Confirm_Content::saveMsg(Message_Ping *msg)
{
    if (msg == nullptr)
    {
        return;
    }

    Settings_Manager::addMessage(msg->status, strlen(msg->status));
}

void Save_Confirm_Content::saveCoordinates(Message_Ping *msg)
{
    return;
    // TODO: Implement coordinate storage
    if (msg == nullptr)
    {
        return;
    }

    // Settings_Manager::addCoordinates(msg->latitude, msg->longitude);
}