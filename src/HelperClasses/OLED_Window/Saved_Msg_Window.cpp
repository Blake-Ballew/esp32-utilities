#include "Saved_Msg_Window.h"

Saved_Msg_Window::Saved_Msg_Window(OLED_Window *parent) : OLED_Window(parent)
{
    this->assignButton(ACTION_NONE, 1, "", 0);
    this->assignButton(ACTION_DEFER_CALLBACK_TO_WINDOW, 2, "Delete", 6);
    this->assignButton(ACTION_BACK, 3, "Back", 4);
    this->assignButton(ACTION_DEFER_CALLBACK_TO_WINDOW, 4, "New Message", 11);

    this->msgContent = new Saved_Messages_Content();
    this->editContent = new Edit_String_Content();
    this->content = this->msgContent;
    this->saveList = false;
}

Saved_Msg_Window::~Saved_Msg_Window()
{
    if (this->content->type == ContentType::EDIT_STRING)
    {
        editContent->stop();
    }
    delete this->msgContent;
    delete this->editContent;
    this->content = nullptr;
}

void Saved_Msg_Window::execBtnCallback(uint8_t inputID)
{
    switch (inputID)
    {
    case 1:
    {
        if (this->content->type == ContentType::EDIT_STRING)
        {
            editContent->passButtonPress(1);
        }
        break;
    }
    case 2:
    {
        if (this->content->type == ContentType::SAVED_MSG)
        {
            msgContent->deleteMsg();
            Display_Utils::clearContentArea();
            display->setCursor(Display_Utils::centerTextHorizontal(15), Display_Utils::centerTextVertical());
            display->print("Message Deleted");
            display->display();
            vTaskDelay(pdMS_TO_TICKS(1000));
            this->content->printContent();
            saveList = true;
        }
        else if (this->content->type == ContentType::EDIT_STRING)
        {
            if (newMsg != nullptr)
            {
                memcpy(newMsg, editContent->getString(), Settings_Manager::maxMsgLength + 1);
                editContent->stop();
#if DEBUG == 1
                Serial.printf("Message Saving: %s\n", newMsg);
#endif
                msgContent->saveMsg(newMsg, Settings_Manager::maxMsgLength);
                editContent->clearString();
                Display_Utils::clearContentArea();
                display->setCursor(Display_Utils::centerTextHorizontal(13), Display_Utils::centerTextVertical());
                display->print("Message Saved");
                display->display();
                saveList = true;
                vTaskDelay(pdMS_TO_TICKS(1000));
            }
            this->content = this->msgContent;
            this->content->printContent();
            this->assignButton(ACTION_NONE, 1, "", 0);
            this->assignButton(ACTION_DEFER_CALLBACK_TO_WINDOW, 2, "Delete", 6);
            this->assignButton(ACTION_BACK, 3, "Back", 4);
            this->assignButton(ACTION_DEFER_CALLBACK_TO_WINDOW, 4, "New Message", 11);
        }
        break;
    }
    case 3:
    {
        if (this->content->type == ContentType::EDIT_STRING)
        {
            this->assignButton(ACTION_NONE, 1, "", 0);
            this->assignButton(ACTION_DEFER_CALLBACK_TO_WINDOW, 2, "Delete", 6);
            this->assignButton(ACTION_BACK, 3, "Back", 4);
            this->assignButton(ACTION_DEFER_CALLBACK_TO_WINDOW, 4, "New Message", 11);
            editContent->stop();
            this->content = this->msgContent;
        }
        else
        {
            if (saveList)
            {
                Settings_Manager::writeMessagesToEEPROM();
                saveList = false;
            }
        }
        break;
    }
    case 4:
    {
        if (this->content->type == ContentType::SAVED_MSG)
        {
            newMsg = new char[Settings_Manager::maxMsgLength + 1];
            memset(newMsg, 0, Settings_Manager::maxMsgLength + 1);
            editContent->setString(newMsg, Settings_Manager::maxMsgLength, 0);
            content = editContent;
            content->printContent();
            assignButton(ACTION_DEFER_CALLBACK_TO_WINDOW, 1, "Backspace", 9);
            assignButton(ACTION_DEFER_CALLBACK_TO_WINDOW, 2, "Save", 4);
            assignButton(ACTION_DEFER_CALLBACK_TO_WINDOW, 3, "Back", 4);
            assignButton(ACTION_DEFER_CALLBACK_TO_WINDOW, 4, "Select", 6);
            editContent->start();
        }
        else if (this->content->type == ContentType::EDIT_STRING)
        {
            editContent->passButtonPress(4);
        }
        break;
    }
    }
}