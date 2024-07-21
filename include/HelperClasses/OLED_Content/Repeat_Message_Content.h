#pragma once

#include "OLED_Content.h"
#include "Navigation_Manager.h"
#include "Network_Manager.h"
#include "LED_Manager.h"
#include "Message_Types.h"

#define Repeat_Message_Content_TIMER_TICK 17
#define Repeat_Message_Content_TICKS_PER_MESSAGE 1800

class Repeat_Message_Content : OLED_Content
{
public:
    bool confirmed;

    // newMsgID is used to generate a new message ID with each broadcast
    Repeat_Message_Content(bool newMsgID);
    ~Repeat_Message_Content();

    void printContent();
    static void updateDisplay(TimerHandle_t timer);

    void Pause();
    void Resume();

    void encUp() {}
    void encDown() {}

    void confirmSOS();
    void unconfirmSOS();

private:
    void sendSOS();
    void sendOkay();

    uint32_t msgID;
};