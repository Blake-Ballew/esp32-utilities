#pragma once

#include "Window.hpp"
#include "States/AwaitWifiState.hpp"
#include "States/BroadcastTcpState.hpp"
#include "DisplayUtilities.hpp"
#include "RadioUtils.h"

namespace DisplayModule
{
    // -------------------------------------------------------------------------
    // WiFiRpcWindow
    // -------------------------------------------------------------------------
    // WiFi RPC server flow:
    //
    //   AwaitWifiState (connect to WiFi)
    //     ↓ WiFi connected (detected each tick)
    //   BroadcastTcpState (broadcast IP:port every 500 ms)
    //     ↓ WiFi lost (detected each tick)
    //   AwaitWifiState (reconnect)
    //
    // Radio is shut down on destruction.
    // BUTTON_3 pops the window from either state.
    //
    // Usage:
    //   Utilities::pushWindow(std::make_shared<WiFiRpcWindow>());

    class WiFiRpcWindow : public Window
    {
    public:
        WiFiRpcWindow()
        {
            _awaitState     = std::make_shared<AwaitWifiState>();
            _broadcastState = std::make_shared<BroadcastTcpState>();

            registerInput(InputID::BUTTON_3, "Back");
            addInputCommand(InputID::BUTTON_3,
                [](const InputContext &) { Utilities::popWindow(); });

            setInitialState(_awaitState);
        }

        ~WiFiRpcWindow()
        {
            if (ConnectivityModule::RadioUtils::IsRadioActive())
                ConnectivityModule::RadioUtils::DisableRadio();
        }

        // Switch states based on WiFi link status each autonomous tick
        void onTick() override
        {
            Window::onTick(); // dispatch to current state

            if (_currentState == _awaitState
                && ConnectivityModule::RadioUtils::IsWiFiActive())
            {
                switchState(_broadcastState);
            }
            else if (_currentState == _broadcastState
                     && !ConnectivityModule::RadioUtils::IsWiFiActive())
            {
                switchState(_awaitState);
            }
        }

    private:
        std::shared_ptr<AwaitWifiState>     _awaitState;
        std::shared_ptr<BroadcastTcpState>  _broadcastState;
    };

} // namespace DisplayModule
