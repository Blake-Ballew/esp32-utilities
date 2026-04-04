#pragma once

#include <vector>
#include <string>
#include "WindowState.hpp"
#include "DisplayUtilities.hpp"
#include "TextDrawCommand.hpp"
#include <ArduinoJson.h>

namespace DisplayModule
{
    // -------------------------------------------------------------------------
    // TextDisplayState
    // -------------------------------------------------------------------------
    // Displays one or more formatted text lines.  Lines can be supplied either
    // programmatically (setLines) or via StateTransferData payload.
    //
    // Payload schema (DynamicJsonDocument):
    //   {
    //     "txtLines": [
    //       {
    //         "text":   "Hello",
    //         "HAlign": 0,        // TextAlignH: 0=LEFT 1=CENTER 2=RIGHT  (default CENTER)
    //         "VAlign": 5,        // TextAlignV: 0=TOP 1=CENTER 2=BOTTOM  (default LINE)
    //         "Line":   2         // 1-based line number, only used when VAlign=LINE
    //       },
    //       ...
    //     ]
    //   }
    //
    // If no payload (or no "txtLines" key), any lines set via setLines() are
    // used.  setLines() can be called at any time; call rebuildDrawCommands()
    // afterwards if the state is already active.

    class TextDisplayState : public WindowState
    {
    public:
        TextDisplayState() = default;

        // ------------------------------------------------------------------
        // Programmatic setup
        // ------------------------------------------------------------------

        void setLines(std::vector<TextDrawData> lines)
        {
            _lines = std::move(lines);
        }

        void addLine(TextDrawData line)
        {
            _lines.push_back(std::move(line));
        }

        void clearLines()
        {
            _lines.clear();
        }

        // Rebuild draw commands to reflect the current _lines vector.
        // Called automatically on enter; call manually if you mutate _lines
        // while the state is active.
        void rebuildDrawCommands()
        {
            _rebuildDrawCommands();
        }

        // ------------------------------------------------------------------
        // Lifecycle
        // ------------------------------------------------------------------

        void onEnter(const StateTransferData &data) override
        {
            // Payload overrides programmatic lines
            if (data.payload && data.payload->containsKey("txtLines"))
            {
                _lines.clear();
                uint16_t autoLine = 2; // default line counter for entries without explicit line

                for (auto lineObj : (*data.payload)["txtLines"].as<ArduinoJson::JsonArray>())
                {
                    TextDrawData tdd;
                    tdd.text = lineObj["text"].as<std::string>();

                    tdd.format.hAlign =
                        lineObj.containsKey("HAlign")
                        ? static_cast<TextAlignH>(lineObj["HAlign"].as<int>())
                        : TextAlignH::CENTER;

                    tdd.format.vAlign =
                        lineObj.containsKey("VAlign")
                        ? static_cast<TextAlignV>(lineObj["VAlign"].as<int>())
                        : TextAlignV::LINE;

                    if (tdd.format.vAlign == TextAlignV::LINE)
                    {
                        tdd.format.line = lineObj.containsKey("Line")
                                          ? lineObj["Line"].as<uint16_t>()
                                          : autoLine;
                    }

                    ++autoLine;
                    _lines.push_back(std::move(tdd));
                }
            }

            _rebuildDrawCommands();
        }

        // ------------------------------------------------------------------
        // Helpers — build and send a payload from a line vector
        // ------------------------------------------------------------------

        static std::shared_ptr<ArduinoJson::DynamicJsonDocument>
        buildPayload(const std::vector<TextDrawData> &lines)
        {
            const size_t capacity = 64 + lines.size() * 128;
            auto doc = std::make_shared<ArduinoJson::DynamicJsonDocument>(capacity);
            auto arr = doc->createNestedArray("txtLines");

            for (const auto &tdd : lines)
            {
                auto obj = arr.createNestedObject();
                obj["text"]   = tdd.text;
                obj["HAlign"] = static_cast<int>(tdd.format.hAlign);
                obj["VAlign"] = static_cast<int>(tdd.format.vAlign);
                obj["Line"]   = tdd.format.line;
            }
            return doc;
        }

    protected:
        std::vector<TextDrawData> _lines;

    private:
        void _rebuildDrawCommands()
        {
            clearDrawCommands();
            for (const auto &tdd : _lines)
            {
                addDrawCommand(std::make_shared<TextDrawCommand>(tdd.text, tdd.format));
            }
        }
    };

} // namespace DisplayModule
