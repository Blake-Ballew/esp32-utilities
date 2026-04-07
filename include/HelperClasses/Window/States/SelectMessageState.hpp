#pragma once

#include <string>
#include <vector>
#include <memory>
#include "WindowState.hpp"
#include "TextDrawCommand.hpp"
#include "LED_Utils.h"
#include "ScrollWheel.hpp"

namespace DisplayModule
{
    // -------------------------------------------------------------------------
    // SelectMessageState
    // -------------------------------------------------------------------------
    // Presents a scrollable list of string messages for the user to select one
    // for sending as a broadcast or direct LoRa message.
    //
    // Payload in:
    //   { "Messages": ["message1", "message2", ...] }
    //
    // Payload out (on BUTTON_4 — Send):
    //   { "Message": "<selected message>" }
    //
    // No payload on BUTTON_3 (Back / cancel).
    //
    // Input layout (wired by owning Window):
    //   ENC_UP / ENC_DOWN — scroll
    //   BUTTON_4          — "Send"  (pop state with selected message)
    //   BUTTON_3          — "Back"  (pop state without payload)

    class SelectMessageState : public WindowState
    {
    public:
        SelectMessageState()
        {
            bindInput(InputID::BUTTON_3, "Back");
            bindInput(InputID::BUTTON_4, "Send");
            bindInput(InputID::ENC_UP, "", [this](const InputContext &) {
                if (_messages.size() <= 1) return;
                if (_it == _messages.begin())
                    _it = _messages.end() - 1;
                else
                    --_it;
                _rebuildDrawCommands();
            });
            bindInput(InputID::ENC_DOWN, "", [this](const InputContext &) {
                if (_messages.size() <= 1) return;
                ++_it;
                if (_it == _messages.end())
                    _it = _messages.begin();
                _rebuildDrawCommands();
            });
        }

        // ------------------------------------------------------------------
        // Lifecycle
        // ------------------------------------------------------------------

        void onEnter(const StateTransferData &data) override
        {
            _messages.clear();

            if (data.payload)
            {
                auto &doc = *data.payload;
                if (doc.containsKey("Messages")
                    && doc["Messages"].is<ArduinoJson::JsonArray>())
                {
                    for (auto msg : doc["Messages"].as<ArduinoJson::JsonArrayConst>())
                    {
                        _messages.push_back(msg.as<std::string>());
                        ESP_LOGV(TAG, "Loaded message option: %s", _messages.back().c_str());
                    } 
                }
                else
                {
                    ESP_LOGW(TAG, "SelectMessageState entered with invalid payload");
                }
            }

            _it            = _messages.begin();
            _scrollWheelID = ScrollWheel::RegisteredPatternID();
            LED_Utils::enablePattern(_scrollWheelID);

            _rebuildDrawCommands();
        }

        void onExit() override
        {
            LED_Utils::disablePattern(_scrollWheelID);
            WindowState::onExit();
        }

        // ------------------------------------------------------------------
        // Payload builder for owning Window (BUTTON_4 handler)
        // ------------------------------------------------------------------

        void buildSelectPayload(JsonDocument &doc) const
        {
            if (_messages.empty() || _it == _messages.end())
                return;

            doc["Message"] = *_it;
        }

        bool hasMessages() const { return !_messages.empty(); }

    private:
        std::vector<std::string>          _messages;
        std::vector<std::string>::iterator _it;
        int _scrollWheelID = -1;

        void _rebuildDrawCommands()
        {
            clearDrawCommands();

            bindInput(InputID::ENC_DOWN, _messages.size() <= 1 ? "" : "v");
            bindInput(InputID::ENC_UP, _messages.size() <= 1 ? "" : "^");

            if (!_messages.empty() && _it != _messages.end())
            {
                addDrawCommand(std::make_shared<TextDrawCommand>(
                    "Select a message",
                    TextFormat{ TextAlignH::CENTER, TextAlignV::LINE, 2 }
                ));
                addDrawCommand(std::make_shared<TextDrawCommand>(
                    *_it,
                    TextFormat{ TextAlignH::CENTER, TextAlignV::LINE, 3 }
                ));

                if (_scrollWheelID >= 0)
                {
                    ArduinoJson::StaticJsonDocument<64> cfg;
                    cfg["numItems"] = _messages.size();
                    cfg["currItem"] = std::distance(_messages.begin(), _it);
                    LED_Utils::configurePattern(_scrollWheelID, cfg);
                    LED_Utils::iteratePattern(_scrollWheelID);
                }
            }
            else
            {
                addDrawCommand(std::make_shared<TextDrawCommand>(
                    "No messages",
                    TextFormat{ TextAlignH::CENTER, TextAlignV::CENTER }
                ));
            }
        }
    };

} // namespace DisplayModule
