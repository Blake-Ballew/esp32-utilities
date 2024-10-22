#pragma once

#include "Window_State.h"

class DiagnosticsState : public Window_State
{
public:
    DiagnosticsState() {}

    void displayState()
    {
        TextFormat format;
        std::string text;
        format.horizontalAlignment = TextAlignmentHorizontal::ALIGN_LEFT;
        format.verticalAlignment = TextAlignmentVertical::TEXT_LINE;
        format.line = 1;

        text = "Heap Free: ";
        std::string heapFreeUnits = "b";
        // Presumably in bytes
        auto freeHeap = ESP.getFreeHeap();
        if (freeHeap > 1024)
        {
            freeHeap >>= 10;
            heapFreeUnits = "Kb";
            if (freeHeap > 1024)
            {
                freeHeap >>= 10;
                heapFreeUnits = "Mb";
            }
        }

        text += std::to_string(freeHeap);
        text += heapFreeUnits;
        Display_Utils::printFormattedText(text.c_str(), format);

        format.line = 2;
        text = "Heap Frag: ";
        text += std::to_string((float)ESP.getMaxAllocHeap() / (float)ESP.getFreeHeap());
        text += "%";
        Display_Utils::printFormattedText(text.c_str(), format);

        // Stack water level
        format.line = 3;
        text = "Stack Left: ";
        text += std::to_string(uxTaskGetStackHighWaterMark(NULL));
        Display_Utils::printFormattedText(text.c_str(), format);

        Display_Utils::UpdateDisplay().Invoke();
        
    }


protected:

};