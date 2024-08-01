#pragma once

#include "OLED_Window.h"
#include "MessagePing.h"
#include "Ping_Content.h"

class Ping_Window : public OLED_Window
{
public:
    Ping_Window(OLED_Window *parent, MessageBase *msg)
    {
        assignButton(ACTION_BACK, BUTTON_3, "Back", 4);
        assignButton(ACTION_DEFER_CALLBACK_TO_WINDOW, BUTTON_4, "Send", 4);
    }

    void execBtnCallback(uint8_t inputID)
    {
        uint8_t callbackID;
        switch (inputID)
        {
        case BUTTON_1:
            callbackID = btn1CallbackID;
            break;
        case BUTTON_2:
        {
            callbackID = btn2CallbackID;
            break;
        }
            // Back button
        case BUTTON_3:
            callbackID = btn3CallbackID;
            break;
            // Send Ping button
        case BUTTON_4:
        {
            callbackID = btn4CallbackID;
            Ping_Content *c = (Ping_Content *)content;
            assignButton(ACTION_NONE, BUTTON_4, "", 0);
            c->sendPing();
            break;
        }
        default:
            break;
        }

        switch (callbackID)
        {
        case ACTION_BACK:

            break;
        default:
            break;
        }
    }
};