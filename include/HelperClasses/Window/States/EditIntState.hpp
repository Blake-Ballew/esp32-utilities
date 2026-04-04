#pragma once

#include <cstdint>
#include <cstdio>
#include <string>
#include "EditStateBase.hpp"
#include "DisplayUtilities.hpp"
#include "TextDrawCommand.hpp"
#include <ArduinoJson.h>

namespace DisplayModule
{
    // -------------------------------------------------------------------------
    // EditIntState
    // -------------------------------------------------------------------------
    // Edits a bounded integer value (signed or unsigned).
    //
    // Payload in (from SettingsState::buildEditPayload):
    //   {
    //     "cfgVal": 42,
    //     "minVal": 0,
    //     "maxVal": 100,
    //     "incVal": 1,
    //     "signed": false
    //   }
    //
    // Payload out (on BUTTON_4 confirm):
    //   { "return": <int> }
    // No payload on BUTTON_3 (cancel).
    //
    // Input wiring (set by SettingsWindow):
    //   ENC_UP   — decrement (wraps at min → max)
    //   ENC_DOWN — increment (wraps at max → min)
    //   BUTTON_3 — cancel
    //   BUTTON_4 — confirm

    class EditIntState : public EditStateBase
    {
    public:
        EditIntState()
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
            _isSigned = true;
            _value    = 0;
            _min      = INT32_MIN;
            _max      = INT32_MAX;
            _step     = 1;

            if (data.payload)
            {
                auto &doc = *data.payload;
                if (doc.containsKey("signed"))
                    _isSigned = doc["signed"].as<bool>();
                if (doc.containsKey("cfgVal"))
                    _value = doc["cfgVal"].as<int32_t>();
                if (doc.containsKey("minVal"))
                    _min = doc["minVal"].as<int32_t>();
                if (doc.containsKey("maxVal"))
                    _max = doc["maxVal"].as<int32_t>();
                if (doc.containsKey("incVal"))
                    _step = doc["incVal"].as<int32_t>();
            }

            _clamp();
            _rebuildDrawCommands();
        }

        // ------------------------------------------------------------------
        // Result
        // ------------------------------------------------------------------

        int32_t currentValue() const { return _value; }

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
        buildInputPayload(int32_t current,
                          int32_t minVal,
                          int32_t maxVal,
                          int32_t step    = 1,
                          bool    isSigned = true)
        {
            auto doc = std::make_shared<ArduinoJson::DynamicJsonDocument>(128);
            (*doc)["cfgVal"] = current;
            (*doc)["minVal"] = minVal;
            (*doc)["maxVal"] = maxVal;
            (*doc)["incVal"] = step;
            (*doc)["signed"] = isSigned;
            return doc;
        }

    private:
        bool    _isSigned = true;
        int32_t _value    = 0;
        int32_t _min      = INT32_MIN;
        int32_t _max      = INT32_MAX;
        int32_t _step     = 1;

        void _clamp()
        {
            if (_value < _min) _value = _min;
            if (_value > _max) _value = _max;
        }

        void _rebuildDrawCommands()
        {
            clearDrawCommands();

            char buf[16];
            snprintf(buf, sizeof(buf), "%ld", static_cast<long>(_value));

            addDrawCommand(std::make_shared<TextDrawCommand>(
                std::string(buf),
                TextFormat{ TextAlignH::CENTER, TextAlignV::CENTER }
            ));
        }
    };

} // namespace DisplayModule
