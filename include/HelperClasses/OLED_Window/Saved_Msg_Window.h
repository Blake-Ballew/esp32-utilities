#pragma once

#include "OLED_Window.h"
#include "Settings_Manager.h"
#include "Saved_Msg_Content.h"
#include "Edit_String_Content.h"

class Saved_Msg_Window : public OLED_Window
{
public:
    Saved_Msg_Window(OLED_Window *parent);
    ~Saved_Msg_Window();

    void execBtnCallback(uint8_t buttonNumber, void *arg);

    const int maxMsgLength = 23;

private:
    bool saveList;
    char *newMsg;

    Saved_Msg_Content *msgContent;
    Edit_String_Content *editContent;
};