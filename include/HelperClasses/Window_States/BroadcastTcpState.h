#pragma once

#include "NetworkUtils.h"
#include "TcpConnectionBroadcast.h"
#include "Window_State.h"

class BroadcastTcpState : public Window_State
{
public:

    BroadcastTcpState() : Window_State() {}

    void displayState()
    {
        _broadcast.SendBroadcast();

        TextFormat prompt;
        std::string message = "TCP Server on:";

        prompt.horizontalAlignment = ALIGN_CENTER_HORIZONTAL;
        prompt.verticalAlignment = TEXT_LINE;
        prompt.line = 1;
        Display_Utils::printFormattedText(message.c_str(), prompt);

        prompt.line = 2;
        message = "IP: ";
        message += WiFi.localIP().toString().c_str();
        Display_Utils::printFormattedText(message.c_str(), prompt);

        prompt.line = 3;
        message = "Port: ";
        message += std::to_string(NetworkModule::Utilities::RPC_PORT);
        Display_Utils::printFormattedText(message.c_str(), prompt);

        Display_Utils::UpdateDisplay().Invoke();
    }

    void enterState(State_Transfer_Data &transferData) 
    {
        if (!_broadcast.EstablishConnection(NetworkModule::Utilities::GetBroadcastIP(), _broadcast.PortToBroadcast()))
        {
            #if DEBUG == 1
            Serial.println("Failed to establish broadcast connection");
            #endif
        }
        else
        {
            #if DEBUG == 1
            Serial.println("Broadcast connection established");
            #endif
        }
        
        _broadcast.SetPortToBroadcast(NetworkModule::Utilities::RPC_PORT);
        Window_State::enterState(transferData);
        Display_Utils::enableRefreshTimer(10000);
        _broadcast.SendBroadcast();
    }

    void exitState(State_Transfer_Data &transferData) 
    {
        Window_State::exitState(transferData);
    }

protected:
    NetworkModule::TcpConnectionBroadcast _broadcast;
};