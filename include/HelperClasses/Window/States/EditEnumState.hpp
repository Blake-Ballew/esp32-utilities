#pragma once

#include <vector>
#include <string>
#include <utility>
#include <cstddef>
#include "EditStateBase.hpp"
#include "DisplayUtilities.hpp"
#include "TextDrawCommand.hpp"
#include <ArduinoJson.h>

namespace DisplayModule
{
    // -------------------------------------------------------------------------
    // EditEnumState
    // -------------------------------------------------------------------------
    // Scrollable enum picker.  The options list is insertion-ordered
    // (vector of pairs) so display order matches registration order.
    //
    // Returns the *index* of the selected option, not its value — consistent
    // with how Settings_Manager maps enum fields back to C++ struct members.
    //
    // Payload in (from SettingsState::buildEditPayload):
    //   {
    //     "valTxt": ["Option A", "Option B", "Option C"],
    //     "cfgVal": 1      // currently selected index
    //   }
    //
    // Payload out (on BUTTON_4 confirm):
    //   { "return": <size_t index> }
    // No payload on BUTTON_3 (cancel).
    //
    // Input wiring (constructor):
    //   ENC_UP   — scroll up   (wraps; no-op when empty)
    //   ENC_DOWN — scroll down (wraps; no-op when empty)
    //   BUTTON_3 — cancel
    //   BUTTON_4 — confirm

    class EditEnumState : public EditStateBase
    {
    public:
        EditEnumState()
        {
            bindInput(InputID::BUTTON_3, "Cancel");
            wireConfirmInput(InputID::BUTTON_4, "Save");
            bindInput(InputID::ENC_UP, "", [this](const InputContext &) {
                if (_options.empty()) return;
                _index = (_index == 0) ? _options.size() - 1 : _index - 1;
                _rebuildDrawCommands();
            });
            bindInput(InputID::ENC_DOWN, "", [this](const InputContext &) {
                if (_options.empty()) return;
                _index = (_index + 1) % _options.size();
                _rebuildDrawCommands();
            });
        }

        // ------------------------------------------------------------------
        // Programmatic option setup (alternative to payload-based init)
        // ------------------------------------------------------------------

        void setOptions(std::vector<std::string> options, size_t selectedIndex = 0)
        {
            _options = std::move(options);
            _index   = (selectedIndex < _options.size()) ? selectedIndex : 0;
        }

        void clearOptions()
        {
            _options.clear();
            _index = 0;
        }

        // ------------------------------------------------------------------
        // Lifecycle
        // ------------------------------------------------------------------

        void onEnter(const StateTransferData &data) override
        {
            _index = 0;

            if (data.payload)
            {
                auto &doc = *data.payload;

                if (doc.containsKey("valTxt"))
                {
                    _options.clear();
                    for (auto txt : doc["valTxt"].as<ArduinoJson::JsonArray>())
                        _options.push_back(txt.as<std::string>());
                }

                if (doc.containsKey("cfgVal"))
                {
                    size_t idx = doc["cfgVal"].as<size_t>();
                    _index = (idx < _options.size()) ? idx : 0;
                }
            }

            _rebuildDrawCommands();
        }

        // ------------------------------------------------------------------
        // Result
        // ------------------------------------------------------------------

        size_t selectedIndex() const { return _index; }

        std::shared_ptr<ArduinoJson::DynamicJsonDocument> buildResultPayload() const override
        {
            auto doc = std::make_shared<ArduinoJson::DynamicJsonDocument>(128);
            (*doc)["return"] = _index;
            return doc;
        }

        // ------------------------------------------------------------------
        // Input payload builder
        // ------------------------------------------------------------------

        static std::shared_ptr<ArduinoJson::DynamicJsonDocument>
        buildInputPayload(const std::vector<std::string> &options,
                          size_t selectedIndex = 0)
        {
            auto doc = std::make_shared<ArduinoJson::DynamicJsonDocument>(
                64 + options.size() * 32
            );

            auto arr = doc->createNestedArray("valTxt");
            for (const auto &opt : options)
                arr.add(opt);

            (*doc)["cfgVal"] = selectedIndex;
            return doc;
        }

    private:
        std::vector<std::string> _options;
        size_t                   _index = 0;

        void _rebuildDrawCommands()
        {
            clearDrawCommands();

            if (_options.empty()) return;

            // Current option — centred
            addDrawCommand(std::make_shared<TextDrawCommand>(
                _options[_index],
                TextFormat{ TextAlignH::CENTER, TextAlignV::CENTER }
            ));
        }
    };

} // namespace DisplayModule
