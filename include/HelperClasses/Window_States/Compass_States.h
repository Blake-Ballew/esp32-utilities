#include "Window_State.h"
#include "Compass_Content.h"

class Compass_Display_State : public Window_State
{
public:
    Compass_Display_State(Compass_Content *compass)
    {
        compassContent = compass;
        renderContent = compass;
        CallbackData backBtn;
        backBtn.callbackID = ACTION_BACK;
        strcpy(backBtn.displayText, "Back");
    }

    ~Compass_Display_State()
    {
        if (compassContent != nullptr)
        {
            compassContent->stop();
        }
    }

    /*
        void enterState(State_Transfer_Data *transferData, Window_State *prevState, Window_State *nextState)
        {
            if (compassContent != nullptr)
            {
                compassContent->start();
            }
        }

        void exitState(State_Transfer_Data *transferData, Window_State *prevState, Window_State *nextState)
        {
            if (compassContent != nullptr)
            {
                compassContent->stop();
            }
        }
        */

    Compass_Content *compassContent;
};