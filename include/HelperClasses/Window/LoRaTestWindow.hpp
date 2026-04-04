#pragma once

#include "Window.hpp"
#include "WindowState.hpp"
#include "TextDrawCommand.hpp"
#include "DisplayUtilities.hpp"
#include "LoraUtils.h"
#include "NavigationUtils.h"
#include "Settings_Manager.h"
#include "MessageBase.h"
#include <string>

namespace DisplayModule
{
    // =========================================================================
    // LoRaTestWindow
    // =========================================================================
    // Diagnostic window for inspecting and sending raw LoRa messages.
    //
    // State machine:
    //   LoRaTestState (the sole state)
    //     ↓ ENC_DOWN    — next message in the received-message list
    //     ↓ ENC_UP      — previous message
    //     ↓ BUTTON_4    — send a test broadcast (base MessageBase)
    //     ↓ BUTTON_3    — Back (pop window)
    //
    // Display:
    //   Line 2: "Name: <senderName>"
    //   Line 3: "MsgID: 0x<hex>"
    //   (or "No messages" centered when the list is empty)
    //
    // Replaces: LoRa_Test_Window.h + LoRa_Test_Content.h

    // -------------------------------------------------------------------------
    // LoRaTestState — inner state, lives entirely inside LoRaTestWindow
    // -------------------------------------------------------------------------
    class LoRaTestState : public WindowState
    {
    public:
        LoRaTestState() = default;

        // ------------------------------------------------------------------
        // Lifecycle
        // ------------------------------------------------------------------

        void onEnter(const StateTransferData &) override
        {
            LoraUtils::ResetMessageIterator();
            _rebuildDrawCommands();
        }

        // ------------------------------------------------------------------
        // Input
        // ------------------------------------------------------------------

        void handleInput(const InputContext &ctx) override
        {
            if (ctx.inputID == InputID::ENC_DOWN)
            {
                if (LoraUtils::GetNumMessages() > 0)
                {
                    LoraUtils::IncrementMessageIterator();
                    _rebuildDrawCommands();
                }
            }
            else if (ctx.inputID == InputID::ENC_UP)
            {
                if (LoraUtils::GetNumMessages() > 0)
                {
                    LoraUtils::DecrementMessageIterator();
                    _rebuildDrawCommands();
                }
            }
        }

    private:
        void _rebuildDrawCommands()
        {
            clearDrawCommands();

            if (LoraUtils::GetNumMessages() == 0)
            {
                addDrawCommand(std::make_shared<TextDrawCommand>(
                    "No messages",
                    TextFormat{ TextAlignH::CENTER, TextAlignV::CENTER }
                ));
                return;
            }

            MessageBase *msg = LoraUtils::GetCurrentMessage();
            if (!msg) return;

            // Line 2: sender name
            std::string nameLine = std::string("Name: ") + msg->senderName;
            addDrawCommand(std::make_shared<TextDrawCommand>(
                nameLine,
                TextFormat{ TextAlignH::LEFT, TextAlignV::LINE, 2 }
            ));

            // Line 3: message ID in hex
            char idBuf[24];
            snprintf(idBuf, sizeof(idBuf), "MsgID: 0x%lX",
                     static_cast<unsigned long>(msg->msgID));
            addDrawCommand(std::make_shared<TextDrawCommand>(
                std::string(idBuf),
                TextFormat{ TextAlignH::LEFT, TextAlignV::LINE, 3 }
            ));

            // LoraUtils::GetCurrentMessage() transfers ownership — free it
            delete msg;
        }
    };

    // =========================================================================
    // LoRaTestWindow
    // =========================================================================
    class LoRaTestWindow : public Window
    {
    public:
        LoRaTestWindow()
        {
            _testState = std::make_shared<LoRaTestState>();

            // ----------------------------------------------------------------
            // BUTTON_3 — Back
            // ----------------------------------------------------------------
            registerInput(InputID::BUTTON_3, "Back");
            addInputCommand(InputID::BUTTON_3,
                [](const InputContext &)
                {
                    Utilities::popWindow();
                });

            // ----------------------------------------------------------------
            // BUTTON_4 — Send test broadcast
            // ----------------------------------------------------------------
            registerInput(InputID::BUTTON_4, "Send");
            addInputCommand(InputID::BUTTON_4,
                [](const InputContext &)
                {
                    uint32_t time = NavigationUtils::GetTime().value();
                    uint32_t date = NavigationUtils::GetDate().value();
                    const char *name =
                        Settings_Manager::settings["User"]["Name"]["cfgVal"]
                            .as<const char *>();

                    MessageBase msg(time, date, /*type=*/0,
                                    LoraUtils::UserID(), name, esp_random());
                    LoraUtils::SendMessage(&msg, /*ttl=*/1);
                });

            setInitialState(_testState);
        }

    private:
        std::shared_ptr<LoRaTestState> _testState;
    };

} // namespace DisplayModule
