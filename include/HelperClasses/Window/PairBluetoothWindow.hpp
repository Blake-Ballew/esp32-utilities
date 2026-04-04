#pragma once

#include "Window.hpp"
#include "WindowState.hpp"
#include "FnDrawCommand.hpp"
#include "TextDrawCommand.hpp"
#include "DisplayUtilities.hpp"
#include "Bluetooth_Utils.h"

namespace DisplayModule
{
    // -------------------------------------------------------------------------
    // PairBluetoothWindow
    // -------------------------------------------------------------------------
    // Shows real-time Bluetooth pairing status at 500 ms refresh:
    //   - Awaiting connection: "Waiting for Connection..."
    //   - Connected, not paired: "PIN: XXXXXX"
    //   - Connected and paired: "Connected!"
    //
    // BUTTON_3 pops the window.
    //
    // Usage:
    //   Utilities::pushWindow(std::make_shared<PairBluetoothWindow>());

    class PairBluetoothWindow : public Window
    {
    public:
        PairBluetoothWindow()
        {
            // Inner state — just a tick-driven display, no lifecycle logic
            class BtState : public WindowState
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

                    if (!Bluetooth_Utils::bluetoothConnected())
                    {
                        addDrawCommand(std::make_shared<TextDrawCommand>(
                            "Waiting for", TextFormat{TextAlignH::CENTER, TextAlignV::LINE, 1}));
                        addDrawCommand(std::make_shared<TextDrawCommand>(
                            "Connection...", TextFormat{TextAlignH::CENTER, TextAlignV::LINE, 2}));
                        addDrawCommand(std::make_shared<TextDrawCommand>(
                            "Visit", TextFormat{TextAlignH::CENTER, TextAlignV::LINE, 4}));
                        addDrawCommand(std::make_shared<TextDrawCommand>(
                            "degen.ammaraskar.com",
                            TextFormat{TextAlignH::CENTER, TextAlignV::LINE, 5}));
                    }
                    else if (!Bluetooth_Utils::bluetoothPaired())
                    {
                        std::string pin = "PIN: "
                            + std::to_string(Bluetooth_Utils::bluetoothPin());
                        addDrawCommand(std::make_shared<TextDrawCommand>(
                            pin,
                            TextFormat{TextAlignH::CENTER, TextAlignV::CENTER}));
                    }
                    else
                    {
                        addDrawCommand(std::make_shared<TextDrawCommand>(
                            "Connected!",
                            TextFormat{TextAlignH::CENTER, TextAlignV::CENTER}));
                    }
                }
            };

            _state = std::make_shared<BtState>();

            registerInput(InputID::BUTTON_3, "Back");
            addInputCommand(InputID::BUTTON_3,
                [](const InputContext &) { Utilities::popWindow(); });

            setInitialState(_state);
        }

    private:
        std::shared_ptr<WindowState> _state;
    };

} // namespace DisplayModule
