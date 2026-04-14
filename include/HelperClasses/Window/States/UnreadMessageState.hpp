#pragma once

#include <memory>
#include <string>
#include <vector>
#include "WindowState.hpp"
#include "TextDrawCommand.hpp"
#include "LoraUtils.h"
#include "NavigationUtils.h"
#include "MessageBase.h"
#include "MessagePing.h"
#include "LED_Utils.h"
#include "SolidRing.hpp"

namespace DisplayModule
{
    // -------------------------------------------------------------------------
    // UnreadMessageState
    // -------------------------------------------------------------------------
    // Steps through all unread LoRa messages.  The current message's printable
    // lines are rendered directly via TextDrawCommands — no OLED_Content
    // subclass involved.
    //
    // Scrolling back past the first message and scrolling forward past the last
    // are both no-ops here; the owning Window checks isAtBeginning() /
    // hasNoUnread() and pops the state as appropriate.
    //
    // Input layout (wired by owning Window):
    //   ENC_UP    — previous message; no-op at first
    //   ENC_DOWN  — next message; no-op at last
    //   BUTTON_3  — mark current message as opened; pop when none remain
    //   BUTTON_4  — "Track"  (Window builds TrackingState payload)
    //   BUTTON_2  — "Reply"  (Window builds reply payload)
    //
    // Payload helpers (called by the owning Window):
    //   buildTrackPayload()  — TrackingState payload from sender's MessagePing
    //   buildReplyPayload()  — { "recipientID": <uint64_t> }
    //
    // LED: SolidRing lit with the current message's color.

    class UnreadMessageState : public WindowState
    {
    public:
        UnreadMessageState()
        {
            bindInput(InputID::BUTTON_3, "Open", [this](const InputContext &) {
                LoraUtils::MarkMessageOpened(
                    LoraUtils::GetCurrentUnreadMessageSenderID());
                if (LoraUtils::IsUnreadMessageIteratorAtEnd() &&
                    LoraUtils::GetNumUnreadMessages() > 0)
                {
                    LoraUtils::DecrementUnreadMessageIterator();
                }
                _currentMsg = LoraUtils::GetCurrentUnreadMessage();
                _configureLed();
                _rebuildDrawCommands();
            });
            bindInput(InputID::BUTTON_2, "Reply");
            bindInput(InputID::BUTTON_4, "Track");
            bindInput(InputID::ENC_UP, "", [this](const InputContext &) {
                if (!LoraUtils::IsUnreadMessageIteratorAtBeginning())
                {
                    LoraUtils::DecrementUnreadMessageIterator();
                    _currentMsg = LoraUtils::GetCurrentUnreadMessage();
                }
                _configureLed();
                _rebuildDrawCommands();
            });
            bindInput(InputID::ENC_DOWN, "", [this](const InputContext &) {
                LoraUtils::IncrementUnreadMessageIterator();
                if (LoraUtils::IsUnreadMessageIteratorAtEnd())
                    LoraUtils::DecrementUnreadMessageIterator();
                else
                    _currentMsg = LoraUtils::GetCurrentUnreadMessage();
                _configureLed();
                _rebuildDrawCommands();
            });
        }

        // ------------------------------------------------------------------
        // Lifecycle
        // ------------------------------------------------------------------

        void onEnter(const StateTransferData &) override
        {
            LoraUtils::ResetUnreadMessageIterator();

            _solidRingID = SolidRing::RegisteredPatternID();

            _currentMsg = LoraUtils::GetCurrentUnreadMessage();
            _configureLed();
            LED_Utils::enablePattern(_solidRingID);

            _rebuildDrawCommands();
        }

        void onExit() override
        {
            _currentMsg = nullptr;
            LED_Utils::disablePattern(_solidRingID);
            LED_Utils::clearPattern(_solidRingID);
            WindowState::onExit();
        }

        // ------------------------------------------------------------------
        // State queries for the owning Window
        // ------------------------------------------------------------------

        bool isAtBeginning() const
        {
            return LoraUtils::IsUnreadMessageIteratorAtBeginning();
        }

        bool hasNoUnread() const
        {
            return LoraUtils::GetNumUnreadMessages() == 0;
        }

        // ------------------------------------------------------------------
        // Payload builders for the owning Window
        // ------------------------------------------------------------------

        // BUTTON_4 (Track) — build TrackingState payload
        std::shared_ptr<ArduinoJson::DynamicJsonDocument> buildTrackPayload() const
        {
            if (!_currentMsg)
                return nullptr;

            auto *ping = static_cast<MessagePing *>(_currentMsg);
            auto  doc  = std::make_shared<ArduinoJson::DynamicJsonDocument>(512);

            (*doc)["lat"]     = ping->lat;
            (*doc)["lon"]     = ping->lng;
            (*doc)["color_R"] = ping->color_R;
            (*doc)["color_G"] = ping->color_G;
            (*doc)["color_B"] = ping->color_B;

            std::vector<MessagePrintInformation> info;
            ping->GetPrintableInformation(info);
            auto arr = (*doc).createNestedArray("displayTxt");
            for (auto &pi : info) arr.add(pi.txt);

            return doc;
        }

        // BUTTON_2 (Reply) — build payload with sender ID
        std::shared_ptr<ArduinoJson::DynamicJsonDocument> buildReplyPayload() const
        {
            if (!_currentMsg) return nullptr;

            auto doc = std::make_shared<ArduinoJson::DynamicJsonDocument>(64);
            (*doc)["recipientID"] = _currentMsg->sender;
            return doc;
        }

    private:
        MessageBase *_currentMsg  = nullptr;
        int          _solidRingID = -1;

        void _configureLed()
        {
            if (_solidRingID < 0 || !_currentMsg) return;
            if (_currentMsg->GetInstanceMessageType() != MessagePing::MessageType())
                return;

            auto *ping = static_cast<MessagePing *>(_currentMsg);
            ArduinoJson::StaticJsonDocument<128> cfg;
            cfg["rOverride"] = ping->color_R;
            cfg["gOverride"] = ping->color_G;
            cfg["bOverride"] = ping->color_B;
            cfg["beginIdx"]  = 0;
            cfg["endIdx"]    = 15;
            LED_Utils::configurePattern(_solidRingID, cfg);
            LED_Utils::iteratePattern(_solidRingID);
        }

        void _rebuildDrawCommands()
        {
            clearDrawCommands();

            if (!_currentMsg)
            {
                addDrawCommand(std::make_shared<TextDrawCommand>(
                    "No Messages",
                    TextFormat{ TextAlignH::CENTER, TextAlignV::CENTER }
                ));
                return;
            }

            // Printable info lines — start at display line 2
            std::vector<MessagePrintInformation> info;
            _currentMsg->GetPrintableInformation(info);

            for (size_t i = 0; i < info.size(); ++i)
            {
                addDrawCommand(std::make_shared<TextDrawCommand>(
                    info[i].txt,
                    TextFormat{ TextAlignH::LEFT, TextAlignV::LINE,
                                static_cast<uint8_t>(i + 2) }
                ));
            }

            // Message age — right-aligned on the first info line (line 2)
            auto ageVal = NavigationUtils::GetTimeDifference(
                _currentMsg->time, _currentMsg->date);
            std::string ageStr = _currentMsg->GetMessageAge(ageVal);
            if (!ageStr.empty())
            {
                addDrawCommand(std::make_shared<TextDrawCommand>(
                    ageStr,
                    TextFormat{ TextAlignH::RIGHT, TextAlignV::LINE, 2 }
                ));
            }
        }
    };

} // namespace DisplayModule
