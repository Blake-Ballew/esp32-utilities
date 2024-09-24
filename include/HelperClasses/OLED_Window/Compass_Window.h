#pragma once

#include "OLED_Window.h"
#include "CompassDebugState.h"
#include "TextDisplayState.h"
#include "CompassCalibrateState.h"

#include "NavigationUtils.h"
#include "LED_Utils.h"

#include "RingPoint.h"

class Compass_Window : public OLED_Window
{
public:
    Compass_Window(OLED_Window *parent) : OLED_Window(parent)
    {
        compassState.assignInput(BUTTON_3, ACTION_BACK, "Back");
        compassState.assignInput(BUTTON_4, ACTION_CALL_FUNCTIONAL_WINDOW_STATE, "Calibrate");
        setInitialState(&compassState);

        textDisplayState.assignInput(BUTTON_3, ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE, "Back");
        textDisplayState.assignInput(BUTTON_4, ACTION_CALL_FUNCTIONAL_WINDOW_STATE, "Confirm");

        std::vector<TextDrawData> textData;
        TextDrawData line1;
        TextDrawData line2;
        TextDrawData line3;

        line1.text = "Rotate device in all";
        line2.text = "directions for 10s";
        line3.text = "to calibrate sensor.";

        line1.format.horizontalAlignment = TextAlignmentHorizontal::ALIGN_CENTER_HORIZONTAL;
        line2.format.horizontalAlignment = TextAlignmentHorizontal::ALIGN_CENTER_HORIZONTAL;
        line3.format.horizontalAlignment = TextAlignmentHorizontal::ALIGN_CENTER_HORIZONTAL;

        line1.format.verticalAlignment = TextAlignmentVertical::TEXT_LINE;
        line2.format.verticalAlignment = TextAlignmentVertical::TEXT_LINE;
        line3.format.verticalAlignment = TextAlignmentVertical::TEXT_LINE;

        line1.format.line = 1;
        line2.format.line = 2;
        line3.format.line = 3;

        textData.push_back(line1);
        textData.push_back(line2);
        textData.push_back(line3);

        textDisplayState.SetFormattedText(textData);

        compassState.setAdjacentState(BUTTON_4, &textDisplayState);
        textDisplayState.setAdjacentState(BUTTON_4, &compassCalibrateState);
    }

    ~Compass_Window()
    {
        Display_Utils::disableRefreshTimer();
    }

    void Pause()
    {

    }

    void Resume()
    {

    }

protected:
    CompassDebugState compassState;
    TextDisplayState textDisplayState;
    CompassCalibrateState compassCalibrateState;
};