#pragma once

#include <vector>
#include <string>
#include <memory>
#include "WindowState.hpp"
#include "TextDrawCommand.hpp"
#include "NavigationUtils.h"
#include "LED_Utils.h"
#include "ScrollWheel.hpp"

namespace DisplayModule
{
    // -------------------------------------------------------------------------
    // SavedLocationsState
    // -------------------------------------------------------------------------
    // Scrollable list of saved GPS locations with edit and track actions.
    //
    // Input layout (wired by owning Window):
    //   ENC_UP / ENC_DOWN — scroll list
    //   BUTTON_3          — "Back" (pop window)
    //   BUTTON_4          — "Edit"   — open EditStringState with name
    //   BUTTON_1          — "Delete" — remove selected location
    //   BUTTON_2          — "Track"  — push TrackingState with location data
    //
    // Return from EditStringState:
    //   payload["return"] = new name string → updates selected location's name
    //
    // Getters used by the owning Window to build state-transition payloads:
    //   selectedName() — current location's name (for EditString prefill)
    //   buildTrackPayload() — builds the payload for TrackingState
    //
    // LED: ScrollWheel pattern, enabled on enter, disabled on exit.

    class SavedLocationsState : public WindowState
    {
    public:
        SavedLocationsState()
        {
            bindInput(InputID::BUTTON_1, "Delete", [this](const InputContext &) {
                size_t count = NavigationUtils::GetSavedLocationsSize();
                if (count == 0) return;
                NavigationUtils::RemoveSavedLocation(_selectedIt);
                if (_selectedIt == NavigationUtils::GetSavedLocationsEnd()
                    && NavigationUtils::GetSavedLocationsSize() > 0)
                {
                    --_selectedIt;
                }
                _rebuildDrawCommands();
            });
            bindInput(InputID::BUTTON_2, "Track");
            bindInput(InputID::BUTTON_3, "Back");
            bindInput(InputID::BUTTON_4, "Edit");
            bindInput(InputID::ENC_UP, "", [this](const InputContext &) {
                size_t count = NavigationUtils::GetSavedLocationsSize();
                if (count == 0) return;
                if (_selectedIt == NavigationUtils::GetSavedLocationsBegin())
                    _selectedIt = NavigationUtils::GetSavedLocationsEnd();
                --_selectedIt;
                _rebuildDrawCommands();
            });
            bindInput(InputID::ENC_DOWN, "", [this](const InputContext &) {
                size_t count = NavigationUtils::GetSavedLocationsSize();
                if (count == 0) return;
                ++_selectedIt;
                if (_selectedIt == NavigationUtils::GetSavedLocationsEnd())
                    _selectedIt = NavigationUtils::GetSavedLocationsBegin();
                _rebuildDrawCommands();
            });
        }

        // ------------------------------------------------------------------
        // Lifecycle
        // ------------------------------------------------------------------

        void onEnter(const StateTransferData &data) override
        {
            // Returned from edit state? Apply the new name.
            if (data.payload && data.payload->containsKey("return"))
            {
                std::string newName = (*data.payload)["return"].as<std::string>();
                if (_selectedIt != NavigationUtils::GetSavedLocationsEnd()
                    && NavigationUtils::GetSavedLocationsSize() > 0)
                {
                    SavedLocation updated = *_selectedIt;
                    updated.Name = newName;
                    NavigationUtils::UpdateSavedLocation(_selectedIt, updated);
                }
            }
            else
            {
                // Fresh entry — reset iterator
                _selectedIt = NavigationUtils::GetSavedLocationsBegin();
            }

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
        // Payload builders — called by the owning Window's input commands
        // ------------------------------------------------------------------

        // For BUTTON_4 (Edit) — sends current name to EditStringState
        std::shared_ptr<ArduinoJson::DynamicJsonDocument> buildEditPayload() const
        {
            auto doc = std::make_shared<ArduinoJson::DynamicJsonDocument>(256);
            if (NavigationUtils::GetSavedLocationsSize() > 0
                && _selectedIt != NavigationUtils::GetSavedLocationsEnd())
            {
                (*doc)["cfgVal"] = _selectedIt->Name;
                (*doc)["maxLen"] = static_cast<int>(STATUS_LENGTH);
            }
            return doc;
        }

        // For BUTTON_2 (Track) — sends target coordinates to TrackingState
        std::shared_ptr<ArduinoJson::DynamicJsonDocument> buildTrackPayload() const
        {
            if (NavigationUtils::GetSavedLocationsSize() == 0
                || _selectedIt == NavigationUtils::GetSavedLocationsEnd())
            {
                return nullptr;
            }

            auto doc = std::make_shared<ArduinoJson::DynamicJsonDocument>(256);
            (*doc)["lat"]     = _selectedIt->Latitude;
            (*doc)["lon"]     = _selectedIt->Longitude;
            (*doc)["color_R"] = LED_Utils::ThemeColor().r;
            (*doc)["color_G"] = LED_Utils::ThemeColor().g;
            (*doc)["color_B"] = LED_Utils::ThemeColor().b;

            auto arr = (*doc).createNestedArray("displayTxt");
            arr.add(_selectedIt->Name);
            return doc;
        }

        bool hasLocations() const
        {
            return NavigationUtils::GetSavedLocationsSize() > 0;
        }

    private:
        std::vector<SavedLocation>::iterator _selectedIt;
        int _scrollWheelID = -1;

        void _rebuildDrawCommands()
        {
            clearDrawCommands();

            size_t count = NavigationUtils::GetSavedLocationsSize();
            if (count > 0
                && _selectedIt != NavigationUtils::GetSavedLocationsEnd())
            {
                addDrawCommand(std::make_shared<TextDrawCommand>(
                    _selectedIt->Name,
                    TextFormat{ TextAlignH::CENTER, TextAlignV::CENTER }
                ));

                if (_scrollWheelID >= 0)
                {
                    ArduinoJson::StaticJsonDocument<64> cfg;
                    cfg["numItems"] = count;
                    cfg["currItem"] = std::distance(
                        NavigationUtils::GetSavedLocationsBegin(), _selectedIt);
                    LED_Utils::configurePattern(_scrollWheelID, cfg);
                    LED_Utils::iteratePattern(_scrollWheelID);
                }
            }
            else
            {
                addDrawCommand(std::make_shared<TextDrawCommand>(
                    "No Saved Locations",
                    TextFormat{ TextAlignH::CENTER, TextAlignV::CENTER }
                ));
            }
        }
    };

} // namespace DisplayModule
