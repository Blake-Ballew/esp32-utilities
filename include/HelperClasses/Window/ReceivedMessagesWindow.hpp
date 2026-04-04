#pragma once

#include "Window.hpp"
#include "States/ReceivedMessagesState.hpp"
#include "States/SelectMessageState.hpp"
#include "States/SelectLocationState.hpp"
#include "States/TrackingState.hpp"
#include "States/DisplaySentMessageState.hpp"
#include "States/UnreadMessageState.hpp"
#include "DisplayUtilities.hpp"
#include "LoraUtils.h"

namespace DisplayModule
{
    // -------------------------------------------------------------------------
    // ReceivedMessagesWindow
    // -------------------------------------------------------------------------
    // Browsing, replying, and tracking from received LoRa messages.
    //
    // State flow:
    //   ReceivedMessagesState (scroll received messages)
    //     ↓ BUTTON_4 ("Track")    → TrackingState with sender GPS data
    //     ↓ BUTTON_2 ("Reply")    → SelectMessageState (pick reply text)
    //       ↓ BUTTON_4 ("Send")   → sends message, pops back
    //     ↓ BUTTON_3 ("Back")     → pop window
    //
    //   TrackingState
    //     ↓ BUTTON_3 ("Back")     → pop state → ReceivedMessagesState
    //
    //
    // Usage:
    //   Utilities::pushWindow(std::make_shared<ReceivedMessagesWindow>());

    class ReceivedMessagesWindow : public Window
    {
    public:
        ReceivedMessagesWindow()
        {
            _receivedState = std::make_shared<ReceivedMessagesState>();
            _trackingState = std::make_shared<TrackingState>();
            _selectMsgState = std::make_shared<SelectMessageState>();

            // ----------------------------------------------------------------
            // BUTTON_3 — context-sensitive Back
            // ----------------------------------------------------------------
            registerInput(InputID::BUTTON_3, "Back");
            addInputCommand(InputID::BUTTON_3,
                [this](const InputContext &ctx)
                {
                    if (_currentState == _receivedState)
                        Utilities::popWindow();
                    else
                        popState();
                });

            // ----------------------------------------------------------------
            // BUTTON_4 — Track sender location
            // ----------------------------------------------------------------
            registerInput(InputID::BUTTON_4, "Track");
            addInputCommand(InputID::BUTTON_4,
                [this](const InputContext &ctx)
                {
                    if (_currentState != _receivedState) return;

                    auto payload = _receivedState->buildTrackPayload();
                    if (!payload) return;

                    StateTransferData d;
                    d.inputID = ctx.inputID;
                    d.payload = payload;
                    pushState(_trackingState, d);
                });

            // ----------------------------------------------------------------
            // BUTTON_2 — Reply: open message selector, then send
            // ----------------------------------------------------------------
            registerInput(InputID::BUTTON_2, "Reply");
            addInputCommand(InputID::BUTTON_2,
                [this](const InputContext &ctx)
                {
                    if (_currentState != _receivedState) return;

                    // Build message list payload from LoraUtils saved messages
                    auto doc = std::make_shared<ArduinoJson::DynamicJsonDocument>(1024);
                    auto arr = (*doc).createNestedArray("Messages");
                    auto it  = LoraUtils::SavedMessageListBegin();
                    while (it != LoraUtils::SavedMessageListEnd())
                    {
                        arr.add(*it);
                        ++it;
                    }

                    StateTransferData d;
                    d.inputID = ctx.inputID;
                    d.payload = doc;
                    pushState(_selectMsgState, d);
                });

            // SelectMessageState: BUTTON_4 = Send (pop with payload → window handles send)
            addInputCommand(InputID::BUTTON_4,
                [this](const InputContext &ctx)
                {
                    if (_currentState != _selectMsgState) return;

                    auto payload = _selectMsgState->buildSelectPayload();
                    if (!payload) return;

                    // Get recipient from the received message display
                    auto replyPayload = _receivedState->buildReplyPayload();
                    uint64_t recipientID = replyPayload
                        ? (*replyPayload)["recipientID"].as<uint64_t>() : 0;

                    // Send the selected message as a direct reply
                    std::string msg = (*payload)["Message"].as<std::string>();
                    LoraUtils::SendDirectMessage(msg, recipientID);

                    popState(); // back to ReceivedMessagesState
                });

            setInitialState(_receivedState);
        }

        ~ReceivedMessagesWindow() = default;

    private:
        std::shared_ptr<ReceivedMessagesState> _receivedState;
        std::shared_ptr<TrackingState>         _trackingState;
        std::shared_ptr<SelectMessageState>    _selectMsgState;
    };

} // namespace DisplayModule
