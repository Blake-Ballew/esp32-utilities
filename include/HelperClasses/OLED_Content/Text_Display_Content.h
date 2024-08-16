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
        #if DEBUG == 1
            Serial.println("Text_Display_Content::printContent()");
        #endif
        for (auto txt : textData)
        {
            Display_Utils::printFormattedText(txt.text.c_str(), txt.format);
            #if DEBUG == 1
                Serial.print("Text_Display_Content::printContent(): ");
                Serial.println(txt.text.c_str());
            #endif
        }
        
        display->display();
    }
protected:

    std::vector<TextDrawData> textData;
};