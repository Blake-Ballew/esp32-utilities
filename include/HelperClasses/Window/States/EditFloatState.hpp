#pragma once

#include <cstdio>
#include <string>
#include "EditStateBase.hpp"
#include "DisplayUtilities.hpp"
#include "TextDrawCommand.hpp"
#include <ArduinoJson.h>

namespace DisplayModule
{
    // -------------------------------------------------------------------------
    // EditFloatState
    // -------------------------------------------------------------------------
    // Edits a bounded floating-point value.
    //
    // Payload in (from SettingsState::buildEditPayload):
    //   {
    //     "cfgVal": 915.0,
    //     "minVal": 850.0,
    //     "maxVal": 950.0,
    //     "incVal": 0.1
    //   }
    //
    // Payload out (on BUTTON_4 confirm):
    //   { "return": <float> }
    // No payload on BUTTON_3 (cancel).
    //
    // Input wiring (set by SettingsWindow):
    //   ENC_UP   — decrement by step (wraps at min → max)
    //   ENC_DOWN — increment by step (wraps at max → min)
    //   BUTTON_3 — cancel
    //   BUTTON_4 — confirm

    class EditFloatState : public EditStateBase
    {
    public:
        EditFloatState()
        {
            bindInput(InputID::BUTTON_3, "Cancel");
            wireConfirmInput(InputID::BUTTON_4, "Save");
            bindInput(InputID::ENC_UP, "", [this](const InputContext &) {
                _value -= _step;
                if (_value < _min) _value = _max;
                _rebuildDrawCommands();
            });
            bindInput(InputID::ENC_DOWN, "", [this](const InputContext &) {
                _value += _step;
                if (_value > _max) _value = _min;
                _rebuildDrawCommands();
            });
        }

        // ------------------------------------------------------------------
        // Lifecycle
        // ------------------------------------------------------------------

        void onEnter(const StateTransferData &data) override
        {
            _value = 0.0f;
            _min   = 0.0f;
            _max   = 0.0f;
            _step  = 0.1f;

            if (data.payload)
            {
                auto &doc = *data.payload;
                if (doc.containsKey("cfgVal")) _value = doc["cfgVal"].as<float>();
                if (doc.containsKey("minVal")) _min   = doc["minVal"].as<float>();
                if (doc.containsKey("maxVal")) _max   = doc["maxVal"].as<float>();
                if (doc.containsKey("incVal")) _step  = doc["incVal"].as<float>();
            }

            _clamp();
            _rebuildDrawCommands();
        }

        // ------------------------------------------------------------------
        // Result
        // ------------------------------------------------------------------

        float currentValue() const { return _value; }

        std::shared_ptr<ArduinoJson::DynamicJsonDocument> buildResultPayload() const override
        {
            auto doc = std::make_shared<ArduinoJson::DynamicJsonDocument>(64);
            (*doc)["return"] = _value;
            return doc;
        }

        // ------------------------------------------------------------------
        // Input payload builder
        // ------------------------------------------------------------------

        static std::shared_ptr<ArduinoJson::DynamicJsonDocument>
        buildInputPayload(float current,
                          float minVal,
                          float maxVal,
                          float step = 0.1f)
        {
            auto doc = std::make_shared<ArduinoJson::DynamicJsonDocument>(256);
            (*doc)["cfgVal"] = current;
            (*doc)["minVal"] = minVal;
            (*doc)["maxVal"] = maxVal;
            (*doc)["incVal"] = step;
            return doc;
        }

    private:
        float _value = 0.0f;
        float _min   = 0.0f;
        float _max   = 0.0f;
        float _step  = 0.1f;

        void _clamp()
        {
            if (_max > _min)
            {
                if (_value < _min) _value = _min;
                if (_value > _max) _value = _max;
            }
        }

        void _rebuildDrawCommands()
        {
            clearDrawCommands();

            char buf[16];
            snprintf(buf, sizeof(buf), "%.2f", static_cast<double>(_value));

            addDrawCommand(std::make_shared<TextDrawCommand>(
                std::string(buf),
                TextFormat{ TextAlignH::CENTER, TextAlignV::CENTER }
            ));
        }
    };

} // namespace DisplayModule
