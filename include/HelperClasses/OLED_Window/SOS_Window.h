#pragma once

#include "OLED_Window.h"
#include "Text_Display_Content.h"
#include "Repeat_Message_State.h"
#include "LoraUtils.h"
#include "Confirm_State.h"
#include "Lock_State.h"
#include "MessagePing.h"

class SOS_Window : public OLED_Window
{
public:
    SOS_Window(OLED_Window *parent, 
    Repeat_Message_State *repeat,
    Lock_State *lock);

    ~SOS_Window();

    void execBtnCallback(uint8_t inputID);
    void Pause();
    void Resume();

    void transferState(State_Transfer_Data &transferData);

protected:
    // Repeat_Message_Content *sosContent;
    Repeat_Message_State *sosState;
    Lock_State *lockState;

    MessagePing *createSosMessage();
    MessagePing *createOkayMessage();
};