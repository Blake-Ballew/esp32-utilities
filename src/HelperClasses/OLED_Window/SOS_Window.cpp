#include "SOS_Window.h"

SOS_Window::SOS_Window(OLED_Window *parent, 
Repeat_Message_State *repeat,
Confirm_State *confirm) : OLED_Window(parent)
{
    sosState = repeat;
    confirmState = confirm;
    
    stateList.push_back(sosState);
    stateList.push_back(confirmState);

    contentList.push_back(sosState->renderContent);
    contentList.push_back(confirmState->renderContent);
}

SOS_Window::~SOS_Window()
{

}

void SOS_Window::execBtnCallback(uint8_t inputID)
{
// #if DEBUG == 1
//     Serial.println("SOS_Window::execBtnCallback");
//     Serial.printf("inputID: %d\n", inputID);
// #endif
//     uint32_t callbackID;
//     switch (inputID)
//     {
//     case BUTTON_1:
//         callbackID = btn1CallbackID;
//         break;
//     case BUTTON_2:
//         callbackID = btn2CallbackID;
//         break;
//     case BUTTON_3:
//     {
//         callbackID = btn3CallbackID;
//         if (callbackID == ACTION_DEFER_CALLBACK_TO_WINDOW)
//         {

//             assignButton(ACTION_BACK, BUTTON_3, "Back", 4);
//             assignButton(ACTION_DEFER_CALLBACK_TO_WINDOW, BUTTON_4, "Confirm", 7);
//             sosContent->unconfirmSOS();
//             LED_Manager::clearRing();
//         }
//         else
//         {

//         }
//     }
//     break;
//     case BUTTON_4:
//     {
//         callbackID = btn4CallbackID;
//         if (sosContent->confirmed == false)
//         {
//             sosContent->confirmSOS();
//             assignButton(ACTION_NONE, BUTTON_4, "\0", 1);
//             assignButton(ACTION_DEFER_CALLBACK_TO_WINDOW, BUTTON_3, "Back", 4);
//         }
//         else
//         {
//         }
//     }
//     break;
//     default:
//         break;
//     }

//     switch (callbackID)
//     {
//     case ACTION_BACK:
//     {
//         LED_Manager::clearRing();
//     }
//     break;
//     default:
//         break;
//     }
}

void SOS_Window::Pause()
{
    sosContent->Pause();
}

void SOS_Window::Resume()
{
    sosContent->Pause();
}
