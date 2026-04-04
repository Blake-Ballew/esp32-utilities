#pragma once

#include "Window.hpp"
#include "States/LockState.hpp"
#include "States/RepeatMessageState.hpp"
#include "DisplayUtilities.hpp"
#include "MessagePing.h"
#include "LoraUtils.h"

namespace DisplayModule
{
    // -------------------------------------------------------------------------
    // SosWindow
    // -------------------------------------------------------------------------
    // SOS / distress beacon flow:
    //
    //   LockState (button sequence gate)
    //     ↓ sequence complete
    //   RepeatMessageState (broadcasts SOS ping every 30 s)
    //
    // The SOS ping is constructed from the last known GPS position and the
    // device's configured theme color.  A new msgID is generated each
    // retransmission so each broadcast has a unique ID.
    //
    // BUTTON_3 in RepeatMessageState exits back to LockState (or pops window).
    //
    // Usage:
    //   Utilities::pushWindow(std::make_shared<SosWindow>());

    class SosWindow : public Window
    {
    public:
        SosWindow()
        {
            _lockState   = std::make_shared<LockState>();
            _repeatState = std::make_shared<RepeatMessageState>();

            // When lock sequence completes, build the SOS ping payload and
            // switch to RepeatMessageState
            _lockState->setOnSequenceComplete(
                [this]()
                {
                    auto payload = _buildSosPingPayload();
                    StateTransferData d;
                    d.payload = payload;
                    switchState(_repeatState, d);
                });

            // BUTTON_3 — exits back to previous window from either state
            registerInput(InputID::BUTTON_3, "Back");
            addInputCommand(InputID::BUTTON_3,
                [this](const InputContext &)
                {
                    if (_currentState == _repeatState)
                        switchState(_lockState);
                    else
                        Utilities::popWindow();
                });

            setInitialState(_lockState);
        }

    private:
        std::shared_ptr<LockState>          _lockState;
        std::shared_ptr<RepeatMessageState> _repeatState;

        // Build a MessagePing payload for the SOS broadcast
        static std::shared_ptr<ArduinoJson::DynamicJsonDocument>
        _buildSosPingPayload()
        {
            // Construct a ping pointing to the current GPS location
            MessagePing ping;
            ping.IsLive  = true; // TrackingState will update coords each tick
            ping.color_R = LED_Utils::ThemeColor().r;
            ping.color_G = LED_Utils::ThemeColor().g;
            ping.color_B = LED_Utils::ThemeColor().b;
            ping.msgID   = esp_random();

            NavigationUtils::UpdateGPS();
            ping.lat  = NavigationUtils::GetLocation().lat();
            ping.lng  = NavigationUtils::GetLocation().lng();
            ping.time = NavigationUtils::GetTime().value();
            ping.date = NavigationUtils::GetDate().value();

            auto doc = std::make_shared<ArduinoJson::DynamicJsonDocument>(512);
            ping.serialize(*doc);
            return doc;
        }
    };

} // namespace DisplayModule
