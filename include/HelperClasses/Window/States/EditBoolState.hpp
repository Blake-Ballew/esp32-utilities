#pragma once

#include "EditStateBase.hpp"
#include "DisplayUtilities.hpp"
#include "TextDrawCommand.hpp"
#include <ArduinoJson.h>

namespace DisplayModule
{
    // -------------------------------------------------------------------------
    // EditBoolState
    // -------------------------------------------------------------------------
    // Edits a single boolean value.
    //
    // Payload in (from SettingsState::buildEditPayload):
    //   { "cfgVal": true|false }
    //
    // Payload out (on BUTTON_4 confirm):
    //   { "return": true|false }
    // No payload on BUTTON_3 (cancel).
    //
    // Input wiring (set by SettingsWindow):
    //   ENC_UP / ENC_DOWN — toggle
    //   BUTTON_3 — cancel (pop state, no payload)
    //   BUTTON_4 — confirm (pop state with payload)

    class EditBoolState : public EditStateBase
    {
    public:
        EditBoolState()
        {
            bindInput(InputID::BUTTON_3, "Cancel");
            wireConfirmInput(InputID::BUTTON_4, "Save");
            bindInput(InputID::ENC_UP, "", [this](const InputContext &) {
                _value = !_value;
                _rebuildDrawCommands();
            });
            bindInput(InputID::ENC_DOWN, "", [this](const InputContext &) {
                _value = !_value;
                _rebuildDrawCommands();
            });
        }

        // ------------------------------------------------------------------
        // Lifecycle
        // ------------------------------------------------------------------

        void onEnter(const StateTransferData &data) override
        {
            ESP_LOGI(TAG, "Entering EditBoolState with payload: %s",
                     data.payload ? data.payload->as<std::string>().c_str() : "null");
            _value = false;

            if (data.payload && data.payload->containsKey("cfgVal"))
                _value = (*data.payload)["cfgVal"].as<bool>();

            _rebuildDrawCommands();
        }

        // ------------------------------------------------------------------
        // Result
        // ------------------------------------------------------------------

        bool currentValue() const { return _value; }

        std::shared_ptr<ArduinoJson::DynamicJsonDocument> buildResultPayload() const override
        {
            ESP_LOGI(TAG, "Building result payload for EditBoolState with value: %s", _value ? "true" : "false");
            auto doc = std::make_shared<ArduinoJson::DynamicJsonDocument>(512);
            (*doc)["return"] = _value;
            return doc;
        }

        // ------------------------------------------------------------------
        // Input payload builder — create a payload to send TO this state
        // ------------------------------------------------------------------

        // static std::shared_ptr<ArduinoJson::DynamicJsonDocument>
        // buildInputPayload(bool currentValue)
        // {
        //     auto doc = std::make_shared<ArduinoJson::DynamicJsonDocument>(64);
        //     (*doc)["cfgVal"] = currentValue;
        //     return doc;
        // }

    private:
        bool _value = false;

        void _rebuildDrawCommands()
        {
            clearDrawCommands();

            addDrawCommand(std::make_shared<TextDrawCommand>(
                _value ? "[ON]" : "[OFF]",
                TextFormat{ TextAlignH::CENTER, TextAlignV::CENTER }
            ));
        }
    };

} // namespace DisplayModule
