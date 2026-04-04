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
#include "SolidRing.h"
#include "ScrollWheel.h"

namespace DisplayModule
{
    // -------------------------------------------------------------------------
    // ReceivedMessagesState
    // -------------------------------------------------------------------------
    // Scrollable list of received LoRa messages.  The current message's
    // printable lines are rendered directly via TextDrawCommands — no
    // OLED_Content subclass involved.
    //
    // Input layout (wired by owning Window):
    //   ENC_UP / ENC_DOWN — scroll through received messages
    //   BUTTON_4          — "Track"  — push TrackingState with sender location
    //   BUTTON_2          — "Reply"  — push SelectMessageState to compose reply
    //   BUTTON_3          — "Back"   — pop window
    //
    // Payload out (on BUTTON_4):
    //   TrackingState payload built from current message's lat/lon/color/displayTxt
    //
    // Payload out (on BUTTON_2):
    //   { "recipientID": <uint64_t sender ID> }
    //
    // LED: SolidRing lit with the current message's color while active.
    //      ScrollWheel shows position in the message list.

    class ReceivedMessagesState : public WindowState
    {
    public:
        ReceivedMessagesState()
        {
            // BUTTON_4 ("Track") requires a Ping message; the Window command
            // guards that, but the label is shown unconditionally as a hint.
            bindInput(InputID::BUTTON_3, "Back");
            bindInput(InputID::BUTTON_2, "Reply");
            bindInput(InputID::BUTTON_4, "Track");
            bindInput(InputID::ENC_UP, "", [this](const InputContext &) {
                if (!LoraUtils::IsReceivedMessageIteratorAtBeginning())
                {
                    LoraUtils::DecrementReceivedMessageIterator();
                    _currentMsg = LoraUtils::GetCurrentReceivedMessage();
                }
                _configureLed();
                _rebuildDrawCommands();
            });
            bindInput(InputID::ENC_DOWN, "", [this](const InputContext &) {
                LoraUtils::IncrementReceivedMessageIterator();
                if (LoraUtils::IsReceivedMessageIteratorAtEnd())
                    LoraUtils::DecrementReceivedMessageIterator();
                else
                    _currentMsg = LoraUtils::GetCurrentReceivedMessage();
                _configureLed();
                _rebuildDrawCommands();
            });
        }

        // ------------------------------------------------------------------
        // Lifecycle
        // ------------------------------------------------------------------

        void onEnter(const StateTransferData &) override
        {
            LoraUtils::ResetReceivedMessageIterator();

            _solidRingID   = SolidRing::RegisteredPatternID();
            _scrollWheelID = ScrollWheel::RegisteredPatternID();

            _currentMsg = LoraUtils::GetCurrentReceivedMessage();
            _configureLed();
            LED_Utils::enablePattern(_solidRingID);

            _rebuildDrawCommands();
        }

        void onExit() override
        {
            LED_Utils::disablePattern(_solidRingID);
            LED_Utils::clearPattern(_solidRingID);
            _currentMsg = nullptr;
            WindowState::onExit();
        }

        void onPause()  override { LED_Utils::disablePattern(_solidRingID); }
        void onResume() override { LED_Utils::enablePattern(_solidRingID);  }

        // ------------------------------------------------------------------
        // Payload builders for the owning Window
        // ------------------------------------------------------------------

        // BUTTON_4 (Track) — build TrackingState payload from current message
        std::shared_ptr<ArduinoJson::DynamicJsonDocument> buildTrackPayload() const
        {
            if (!_currentMsg ||
                _currentMsg->GetInstanceMessageType() != MessagePing::MessageType())
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
        MessageBase *_currentMsg    = nullptr;
        int          _solidRingID  = -1;
        int          _scrollWheelID = -1;

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
