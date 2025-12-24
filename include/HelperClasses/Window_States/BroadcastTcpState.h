#pragma once

#include "IpUtils.h"
#include "TcpConnectionBroadcast.h"
#include "Window_State.h"

namespace
{
    static const char *TAG = "BroadcastTcpState";
}

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
        message += std::to_string(ConnectivityModule::IpUtils::RPC_PORT);
        Display_Utils::printFormattedText(message.c_str(), prompt);

        Display_Utils::UpdateDisplay().Invoke();
    }

    void enterState(State_Transfer_Data &transferData) 
    {
        if (!_broadcast.EstablishConnection(ConnectivityModule::IpUtils::GetWiFiBroadcastIP(), _broadcast.PortToBroadcast()))
        {
            ESP_LOGE(TAG, "Failed to establish broadcast connection");
        }
        else
        {
            ESP_LOGI(TAG, "Broadcast connection established");
        }
        
        _broadcast.SetPortToBroadcast(ConnectivityModule::IpUtils::RPC_PORT);
        Window_State::enterState(transferData);
        Display_Utils::enableRefreshTimer(500);
        _broadcast.SendBroadcast();
    }

    void exitState(State_Transfer_Data &transferData) 
    {
        Window_State::exitState(transferData);
    }

protected:
    ConnectivityModule::TcpConnectionBroadcast _broadcast;
};