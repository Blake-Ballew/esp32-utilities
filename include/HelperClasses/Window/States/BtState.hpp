#pragma once

#include "WindowState.hpp"

namespace DisplayModule
{
    class BtState : public WindowState
    {
    public:
        static constexpr uint32_t REFRESH_MS = 500;

        BtState()
        {
            refreshIntervalMs = REFRESH_MS;
        }

        void onEnter(const StateTransferData &) override 
        { 
            Bluetooth_Utils::initBluetooth();
            _rebuild(); 
        }

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
}