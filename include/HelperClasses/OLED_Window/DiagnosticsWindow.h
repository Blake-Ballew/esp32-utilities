#pragma once 

#include "OLED_Window.h"
#include "DiagnosticsState.h"

class DiagnosticsWindow : public OLED_Window
{
public:

    DiagnosticsWindow(OLED_Window *parent) : OLED_Window(parent) 
    {
        setInitialState(&diagnosticsState);
        diagnosticsState.assignInput(BUTTON_3, ACTION_BACK, "Back");
    }

protected:
    DiagnosticsState diagnosticsState;
};