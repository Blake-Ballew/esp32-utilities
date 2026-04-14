#pragma once

#include <string>
#include <vector>
#include <memory>
#include "WindowState.hpp"
#include "TextDrawCommand.hpp"
#include "LoraUtils.h"
#include "LED_Utils.h"
#include "ScrollWheel.hpp"

namespace DisplayModule
{
    // -------------------------------------------------------------------------
    // SavedStatusMsgListState
    // -------------------------------------------------------------------------
    // Scrollable list of saved LoRa status messages with edit and create actions.
    //
    // Input layout (wired by owning Window):
    //   ENC_UP / ENC_DOWN — scroll list
    //   BUTTON_3          — "Back"   (pop window)
    //   BUTTON_4          — "Edit"   (push EditStringState with current message)
    //   BUTTON_2          — "Create" (push EditStringState with empty message)
    //   BUTTON_1          — "Delete" (remove current message)
    //
    // Return from EditStringState:
    //   payload["return"] → replaces the edited message text (BUTTON_4 path)
    //                        or creates a new entry (BUTTON_2 path)
    //
    // LED: ScrollWheel pattern, enabled on enter, disabled on exit.

    class SavedStatusMsgListState : public WindowState
    {
    public:
        // Tracks which action triggered the push to EditStringState
        enum class PendingAction { None, Edit, Create };

        SavedStatusMsgListState()
        {
            bindInput(InputID::BUTTON_1, "Delete", [this](const InputContext &) {
                LoraUtils::DeleteSavedMessage(_selectedIt);
                if (_selectedIt == LoraUtils::SavedMessageListEnd()
                    && LoraUtils::GetSavedMessageListSize() > 0)
                {
                    --_selectedIt;
                }
                _rebuildDrawCommands();
            });
            bindInput(InputID::BUTTON_2, "Create");
            bindInput(InputID::BUTTON_3, "Back");
            bindInput(InputID::BUTTON_4, "Edit");
            bindInput(InputID::ENC_UP, "", [this](const InputContext &) {
                if (LoraUtils::GetSavedMessageListSize() > 0)
                {
                    if (_selectedIt == LoraUtils::SavedMessageListBegin())
                        _selectedIt = LoraUtils::SavedMessageListEnd();
                    --_selectedIt;
                }
                _rebuildDrawCommands();
            });
            bindInput(InputID::ENC_DOWN, "", [this](const InputContext &) {
                if (LoraUtils::GetSavedMessageListSize() > 0)
                {
                    ++_selectedIt;
                    if (_selectedIt == LoraUtils::SavedMessageListEnd())
                        _selectedIt = LoraUtils::SavedMessageListBegin();
                }
                _rebuildDrawCommands();
            });
        }

        // ------------------------------------------------------------------
        // Lifecycle
        // ------------------------------------------------------------------

        void onEnter(const StateTransferData &data) override
        {
            if (data.payload && data.payload->containsKey("return"))
            {
                std::string newStr = (*data.payload)["return"].as<std::string>();

                if (_pendingAction == PendingAction::Edit
                    && _selectedIt != LoraUtils::SavedMessageListEnd())
                {
                    LoraUtils::UpdateSavedMessage(_selectedIt, newStr);
                }
                else if (_pendingAction == PendingAction::Create)
                {
                    LoraUtils::AddSavedMessage(newStr);
                    _selectedIt = LoraUtils::SavedMessageListBegin(); // reset
                }
            }
            else
            {
                _selectedIt = LoraUtils::SavedMessageListBegin();
            }

            _pendingAction = PendingAction::None;
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
        // Payload builders for the owning Window
        // ------------------------------------------------------------------

        // For BUTTON_4 (Edit) — pre-fills EditStringState with current message
        std::shared_ptr<ArduinoJson::DynamicJsonDocument> buildEditPayload()
        {
            _pendingAction = PendingAction::Edit;
            auto doc = std::make_shared<ArduinoJson::DynamicJsonDocument>(256);
            if (LoraUtils::GetSavedMessageListSize() > 0
                && _selectedIt != LoraUtils::SavedMessageListEnd())
            {
                (*doc)["cfgVal"] = *_selectedIt;
            }
            (*doc)["maxLen"] = static_cast<int>(STATUS_LENGTH);
            return doc;
        }

        // For BUTTON_2 (Create) — opens EditStringState with empty string
        std::shared_ptr<ArduinoJson::DynamicJsonDocument> buildCreatePayload()
        {
            _pendingAction = PendingAction::Create;
            auto doc = std::make_shared<ArduinoJson::DynamicJsonDocument>(64);
            (*doc)["maxLen"] = static_cast<int>(STATUS_LENGTH);
            return doc;
        }

    private:
        std::vector<std::string>::iterator _selectedIt;
        int                                _scrollWheelID = -1;
        PendingAction                      _pendingAction = PendingAction::None;

        void _rebuildDrawCommands()
        {
            clearDrawCommands();

            size_t count = LoraUtils::GetSavedMessageListSize();
            if (count > 0 && _selectedIt != LoraUtils::SavedMessageListEnd())
            {
                addDrawCommand(std::make_shared<TextDrawCommand>(
                    *_selectedIt,
                    TextFormat{ TextAlignH::CENTER, TextAlignV::CENTER }
                ));

                if (_scrollWheelID >= 0)
                {
                    ArduinoJson::StaticJsonDocument<64> cfg;
                    cfg["numItems"] = count;
                    cfg["currItem"] = std::distance(
                        LoraUtils::SavedMessageListBegin(), _selectedIt);
                    LED_Utils::configurePattern(_scrollWheelID, cfg);
                    LED_Utils::iteratePattern(_scrollWheelID);
                }
            }
            else
            {
                addDrawCommand(std::make_shared<TextDrawCommand>(
                    "No Saved Messages",
                    TextFormat{ TextAlignH::CENTER, TextAlignV::CENTER }
                ));
            }
        }
    };

} // namespace DisplayModule
