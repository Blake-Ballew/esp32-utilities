#pragma once

#include "Window.hpp"
#include "States/AwaitWifiState.hpp"
#include "TextDrawCommand.hpp"
#include "DisplayUtilities.hpp"
#include "System_Utils.h"
#include "RadioUtils.h"

namespace DisplayModule
{
    // -------------------------------------------------------------------------
    // OtaUpdateWindow
    // -------------------------------------------------------------------------
    // OTA firmware update flow:
    //
    //   AwaitWifiState (connect to WiFi)
    //     ↓ WiFi connected (detected each tick)
    //   "Connected" state — displays IP address for OTA upload
    //     ↓ WiFi lost (detected each tick)
    //   AwaitWifiState (reconnect)
    //
    // System_Utils::startOTA() is called on construction;
    // System_Utils::stopOTA() and radio cleanup on destruction.
    //
    // BUTTON_3 pops the window from either state.
    //
    // Usage:
    //   Utilities::pushWindow(std::make_shared<OtaUpdateWindow>());

    class OtaUpdateWindow : public Window
    {
    public:
        OtaUpdateWindow()
        {
            System_Utils::startOTA();

            _awaitState = std::make_shared<AwaitWifiState>();

            // "Connected" state — shows local IP
            class ConnectedState : public WindowState
            {
            public:
                static constexpr uint32_t REFRESH_MS = 500;
                uint32_t refreshIntervalMs() const override { return REFRESH_MS; }

                void onEnter(const StateTransferData &) override { _rebuild(); }
                void onTick() override { _rebuild(); }

            private:
                void _rebuild()
                {
                    clearDrawCommands();
                    addDrawCommand(std::make_shared<TextDrawCommand>(
                        "Waiting for firmware",
                        TextFormat{TextAlignH::LEFT, TextAlignV::LINE, 1}));
                    std::string ip = "IP: "
                        + std::string(System_Utils::getLocalIP());
                    addDrawCommand(std::make_shared<TextDrawCommand>(
                        ip,
                        TextFormat{TextAlignH::LEFT, TextAlignV::LINE, 2}));
                }
            };

            _connectedState = std::make_shared<ConnectedState>();

            // BUTTON_3 — back from either state
            registerInput(InputID::BUTTON_3, "Back");
            addInputCommand(InputID::BUTTON_3,
                [](const InputContext &) { Utilities::popWindow(); });

            // Each tick: switch states based on WiFi status
            addInputCommand(InputID::BUTTON_3, // tick is autonomous — use onTick override
                [](const InputContext &) {});

            setInitialState(_awaitState);
        }

        ~OtaUpdateWindow()
        {
            System_Utils::stopOTA();
            if (ConnectivityModule::RadioUtils::IsRadioActive())
                ConnectivityModule::RadioUtils::DisableRadio();
        }

        // Check WiFi state each autonomous refresh tick and switch states
        void onTick() override
        {
            Window::onTick(); // dispatch to current state

            // Switch between await-wifi and connected states based on link
            if (_currentState == _awaitState
                && ConnectivityModule::RadioUtils::IsWiFiActive())
            {
                switchState(_connectedState);
            }
            else if (_currentState == _connectedState
                     && !ConnectivityModule::RadioUtils::IsWiFiActive())
            {
                switchState(_awaitState);
            }
        }

    private:
        std::shared_ptr<AwaitWifiState> _awaitState;
        std::shared_ptr<WindowState>    _connectedState;
    };

} // namespace DisplayModule
