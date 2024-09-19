#pragma once

#include "Window_State.h"
#include <vector>
#include <string>

class TextDisplayState : public Window_State
{
public:
    TextDisplayState() {}
    ~TextDisplayState() {}

    void enterState(State_Transfer_Data &transferData)
    {
        Window_State::enterState(transferData);

        if (transferData.serializedData != nullptr &&
            transferData.serializedData->containsKey("txtLines"))
            {
                _TxtLines.clear();
                size_t txtLine = 2;
                for (auto line : (*transferData.serializedData)["txtLines"].as<JsonArray>())
                {
                    TextDrawData tdd;

                    tdd.text = line["text"].as<std::string>();
                    
                    if (line.containsKey("HAlign"))
                    {
                        tdd.format.horizontalAlignment = (TextAlignmentHorizontal)line["HAlign"].as<int>();
                    }
                    else
                    {
                        tdd.format.horizontalAlignment = TextAlignmentHorizontal::ALIGN_CENTER_HORIZONTAL;
                    }

                    if (line.containsKey("VAlign"))
                    {
                        tdd.format.verticalAlignment = (TextAlignmentVertical)line["VAlign"].as<int>();
                    }
                    else
                    {
                        tdd.format.verticalAlignment = TextAlignmentVertical::TEXT_LINE;
                    }

                    if (tdd.format.verticalAlignment == TextAlignmentVertical::TEXT_LINE &&
                        line.containsKey("Line"))
                        {
                            tdd.format.line = line["Line"].as<int>();
                        }
                    else
                    {
                        tdd.format.line = txtLine;
                    }

                    txtLine++;

                    _TxtLines.push_back(tdd);
                }
            }
    }

    void exitState(State_Transfer_Data &transferData)
    {
        Window_State::exitState(transferData);
    }

    void displayState()
    {
        Window_State::displayState();

        for (auto line : _TxtLines)
        {
            Display_Utils::printFormattedText(line.text.c_str(), line.format);
        }
    }

    void SetFormattedText(std::vector<TextDrawData> &txtLines)
    {
        _TxtLines = txtLines;
    }

protected:
    std::vector<TextDrawData> _TxtLines;
};