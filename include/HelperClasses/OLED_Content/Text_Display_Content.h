#pragma once

#include "OLED_Content.h"

class Text_Display_Content : public OLED_Content
{
public:
    Text_Display_Content(std::vector<TextDrawData> txtData)
    {
        this->textData = txtData;
    }

    void printContent()
    {
        for (auto txt : textData)
        {
            Display_Utils::printFormattedText(txt.text, txt.format);
        }
        
        display->display();
    }
protected:

    std::vector<TextDrawData> textData;
};