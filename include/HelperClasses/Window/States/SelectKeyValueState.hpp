#pragma once

#include <vector>
#include <string>
#include <utility>
#include "WindowState.hpp"
#include "DisplayUtilities.hpp"
#include "TextDrawCommand.hpp"
#include <ArduinoJson.h>

namespace DisplayModule
{
    // -------------------------------------------------------------------------
    // SelectKeyValueState
    // -------------------------------------------------------------------------
    // Scrollable list where each item has a display label (key) and an integer
    // value.  Presents one item at a time; the user scrolls with ENC_UP/ENC_DOWN
    // and confirms with BUTTON_4.
    //
    // On enter (via payload):
    //   {
    //     "prompt": "Choose speed",   // optional header text
    //     "items": [
    //       { "key": "Slow",  "value": 0 },
    //       { "key": "Fast",  "value": 1 }
    //     ]
    //   }
    //
    // On exit (BUTTON_4 confirmed):
    //   payload = { "return": <int value of selected item> }
    //   If the user exits via BUTTON_3 (Back), no payload is set.
    //
    // Wiring in a Window:
    //   win.addInputCommand(InputID::BUTTON_4, [skv](auto &ctx) {
    //       StateTransferData d;
    //       d.inputID = ctx.inputID;
    //       d.payload = skv->buildResultPayload();   // nullptr if back
    //       win.popState(d);
    //   });

    class SelectKeyValueState : public WindowState
    {
    public:
        using Item = std::pair<std::string, int>;

        SelectKeyValueState()
        {
            bindInput(InputID::BUTTON_3, "Back");
            bindInput(InputID::BUTTON_4, "Select");
            bindInput(InputID::ENC_UP, "^", [this](const InputContext &) {
                if (_items.empty()) return;
                _index = (_index == 0) ? _items.size() - 1 : _index - 1;
                _rebuildDrawCommands();
            });
            bindInput(InputID::ENC_DOWN, "v", [this](const InputContext &) {
                if (_items.empty()) return;
                _index = (_index + 1) % _items.size();
                _rebuildDrawCommands();
            });
        }

        // ------------------------------------------------------------------
        // Programmatic setup (alternative to payload-based init)
        // ------------------------------------------------------------------

        void setPrompt(std::string prompt)
        {
            _prompt = std::move(prompt);
        }

        void addItem(std::string key, int value)
        {
            _items.emplace_back(std::move(key), value);
            if (_items.size() == 1) _index = 0;
        }

        void clearItems()
        {
            _items.clear();
            _index = 0;
        }

        size_t itemCount()    const { return _items.size(); }
        size_t currentIndex() const { return _index; }

        // ------------------------------------------------------------------
        // Lifecycle
        // ------------------------------------------------------------------

        void onEnter(const StateTransferData &data) override
        {
            _index = 0;

            if (data.payload)
            {
                auto &doc = *data.payload;

                if (doc.containsKey("prompt"))
                    _prompt = doc["prompt"].as<std::string>();

                if (doc.containsKey("items"))
                {
                    _items.clear();
                    for (auto item : doc["items"].as<ArduinoJson::JsonArray>())
                    {
                        _items.emplace_back(
                            item["key"].as<std::string>(),
                            item["value"].as<int>()
                        );
                    }
                }
            }

            _rebuildDrawCommands();
        }

        // ------------------------------------------------------------------
        // Result — call from the Window's BUTTON_4 onInputCommand
        // ------------------------------------------------------------------

        // Returns a payload with the selected value, or nullptr if nothing selected.
        std::shared_ptr<ArduinoJson::DynamicJsonDocument> buildResultPayload() const
        {
            if (_items.empty()) return nullptr;
            auto doc = std::make_shared<ArduinoJson::DynamicJsonDocument>(64);
            (*doc)["return"] = _items[_index].second;
            return doc;
        }

        int selectedValue() const
        {
            return _items.empty() ? -1 : _items[_index].second;
        }

        const std::string &selectedKey() const
        {
            static const std::string empty;
            return _items.empty() ? empty : _items[_index].first;
        }

        // ------------------------------------------------------------------
        // Payload builder — create a payload to send TO this state
        // ------------------------------------------------------------------

        static std::shared_ptr<ArduinoJson::DynamicJsonDocument>
        buildInputPayload(const std::string &prompt,
                          const std::vector<Item> &items)
        {
            auto doc = std::make_shared<ArduinoJson::DynamicJsonDocument>(
                128 + items.size() * 64
            );
            (*doc)["prompt"] = prompt;
            auto arr = doc->createNestedArray("items");
            for (const auto &[key, val] : items)
            {
                auto obj = arr.createNestedObject();
                obj["key"]   = key;
                obj["value"] = val;
            }
            return doc;
        }

    private:
        std::string       _prompt;
        std::vector<Item> _items;
        size_t            _index = 0;

        void _rebuildDrawCommands()
        {
            clearDrawCommands();

            // Prompt — line 2 (above the item)
            if (!_prompt.empty())
            {
                addDrawCommand(std::make_shared<TextDrawCommand>(
                    _prompt,
                    TextFormat{ TextAlignH::CENTER, TextAlignV::LINE, 2 }
                ));
            }

            // Current key — line 3
            if (!_items.empty())
            {
                addDrawCommand(std::make_shared<TextDrawCommand>(
                    _items[_index].first,
                    TextFormat{ TextAlignH::CENTER, TextAlignV::LINE, 3 }
                ));

            }
        }
    };

} // namespace DisplayModule
