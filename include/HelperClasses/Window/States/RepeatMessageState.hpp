#pragma once

#include <string>
#include <vector>
#include <memory>
#include "WindowState.hpp"
#include "TextDrawCommand.hpp"
#include "LoraUtils.h"
#include "MessagePing.h"
#include "NavigationUtils.h"
#include "LED_Utils.h"
#include "RingPulse.hpp"

namespace DisplayModule
{
    // -------------------------------------------------------------------------
    // RepeatMessageState
    // -------------------------------------------------------------------------
    // Periodically retransmits a LoRa message and drives a Ring_Pulse LED
    // pattern to indicate the broadcast.
    //
    // Payload in (on enter):
    //   Serialized MessageBase-derived object (produced by a previous state).
    //   If the message is a MessagePing with IsLive=true, GPS coordinates are
    //   refreshed on each transmission.
    //
    // Refresh interval = MESSAGE_REPEAT_INTERVAL_MS (30 s default).
    // _allowInterrupts = false.
    //
    // BUTTON_3 (Back) should be wired by the owning Window.
    //
    // LED lifecycle:
    //   onEnter / onResume — enable + loop Ring_Pulse
    //   onPause  / onExit  — disable + clear Ring_Pulse

    class RepeatMessageState : public WindowState
    {
    public:
        static constexpr uint32_t MESSAGE_REPEAT_INTERVAL_MS = 15000;
        static constexpr uint32_t LED_ANIMATION_MS = 3000;

        RepeatMessageState()
        {
        }

        ~RepeatMessageState()
        {
            _deleteMessage();
            _releaseLed();
        }

        // ------------------------------------------------------------------
        // Lifecycle
        // ------------------------------------------------------------------

        void onEnter(const StateTransferData &data) override
        {
            _deleteMessage();

            _ringPulseID = RingPulse::RegisteredPatternID();

            if (data.payload)
            {
                auto &doc  = *data.payload;
                auto  type = MessageBase::GetMessageTypeFromJson(doc);

                if (type == MessagePing::MessageType())
                {
                    auto *ping = new MessagePing();
                    ping->deserialize(doc);
                    _message = ping;

                    // Configure Ring_Pulse colour from message
                    ArduinoJson::StaticJsonDocument<200> cfg;
                    cfg["rOverride"] = ping->color_R;
                    cfg["gOverride"] = ping->color_G;
                    cfg["bOverride"] = ping->color_B;

                    LED_Utils::setAnimationLengthMS(_ringPulseID, LED_ANIMATION_MS);
                    LED_Utils::configurePattern(_ringPulseID, cfg);
                }
            }

            if (_message)
            {
                LED_Utils::enablePattern(_ringPulseID);
                LED_Utils::loopPattern(_ringPulseID, -1);
                refreshIntervalMs = MESSAGE_REPEAT_INTERVAL_MS;
            }
            else
            {
                refreshIntervalMs = 0;
            }

            _rebuildDrawCommands();
        }

        void onExit() override
        {
            _releaseLed();
            _deleteMessage();
            WindowState::onExit();
        }

        void onPause() override
        {
            LED_Utils::clearPattern(_ringPulseID);
        }

        void onResume() override
        {
            if (_message)
                LED_Utils::loopPattern(_ringPulseID, -1);
        }

        // ------------------------------------------------------------------
        // Tick — retransmit message
        // ------------------------------------------------------------------

        void onTick() override
        {
            if (!_message) return;

            // If this is a live ping, update GPS before sending
            if (_message->GetInstanceMessageType() == MessagePing::MessageType())
            {
                auto *ping = static_cast<MessagePing *>(_message);
                if (ping->IsLive)
                {
                    NavigationUtils::UpdateGPS();
                    ping->lat  = NavigationUtils::GetLocation().lat();
                    ping->lng  = NavigationUtils::GetLocation().lng();
                    ping->time = NavigationUtils::GetTime().value();
                    ping->date = NavigationUtils::GetDate().value();
                }
            }

            LoraUtils::SendMessage(_message, 1);
            _rebuildDrawCommands();
        }

    private:
        MessageBase *_message    = nullptr;
        int          _ringPulseID = -1;

        void _deleteMessage()
        {
            delete _message;
            _message = nullptr;
        }

        void _releaseLed()
        {
            if (_ringPulseID >= 0)
            {
                LED_Utils::disablePattern(_ringPulseID);
                LED_Utils::loopPattern(_ringPulseID, 0);
                LED_Utils::resetPattern(_ringPulseID);
            }
        }

        void _rebuildDrawCommands()
        {
            clearDrawCommands();

            if (!_message)
            {
                addDrawCommand(std::make_shared<TextDrawCommand>(
                    "No message",
                    TextFormat{ TextAlignH::CENTER, TextAlignV::CENTER }
                ));
                return;
            }

            auto pingMsg = static_cast<MessagePing *>(_message);

            addDrawCommand(std::make_shared<TextDrawCommand>(
                "Retransmitting...",
                TextFormat{ TextAlignH::CENTER, TextAlignV::LINE, 2 }
            ));

            addDrawCommand(std::make_shared<TextDrawCommand>(
                pingMsg->status,
                TextFormat{ TextAlignH::CENTER, TextAlignV::LINE, 3 }
            ));
        }
    };

} // namespace DisplayModule
