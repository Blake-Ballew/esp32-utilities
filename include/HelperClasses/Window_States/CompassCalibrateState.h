#pragma once

#include "Window_State.h"

#include "NavigationUtils.h"
#include "FilesystemUtils.h"

namespace
{
    const size_t COMPASS_CALIBRATE_WINDOW_REFRESH_RATE_MS = 30;
    const size_t COMPASS_CALIBRATE_WINDOW_TOTAL_TIME_MS = 10000;
}

class CompassCalibrateState : public Window_State
{
public:
    CompassCalibrateState() {}
    ~CompassCalibrateState() {}

    void enterState(State_Transfer_Data &transferData)
    {
        Window_State::enterState(transferData);

        NavigationUtils::BeginCalibration();

        Display_Utils::enableRefreshTimer(COMPASS_CALIBRATE_WINDOW_REFRESH_RATE_MS);

        _LastFrameTime = xTaskGetTickCount();
        _CalibrationTimer = COMPASS_CALIBRATE_WINDOW_TOTAL_TIME_MS;
    }

    void exitState(State_Transfer_Data &transferData)
    {
        Window_State::exitState(transferData);
        Display_Utils::disableRefreshTimer();

        StaticJsonDocument<128> calibrationData;

        NavigationUtils::EndCalibration();
        NavigationUtils::GetCalibrationData(calibrationData);

        std::string buf;
        serializeJson(calibrationData, buf);
        ESP_LOGI(TAG, "Calibration data: %s", buf.c_str());

        FilesystemModule::Utilities::WriteFile(NavigationUtils::GetCalibrationFilename(), calibrationData);
    }

    void displayState()
    {
        Window_State::displayState();

        NavigationUtils::IterateCalibration();
        _CalibrationTimer -= xTaskGetTickCount() - _LastFrameTime;
        _LastFrameTime = xTaskGetTickCount();

        if (_CalibrationTimer <= 0)
        {
            Display_Utils::sendCallbackCommand(ACTION_RETURN_FROM_FUNCTIONAL_WINDOW_STATE);
            return;
        }

        TextDrawData tdd;
        tdd.text = "Calibrating...";

        tdd.format.horizontalAlignment = TextAlignmentHorizontal::ALIGN_CENTER_HORIZONTAL;
        tdd.format.verticalAlignment = TextAlignmentVertical::TEXT_LINE;
        tdd.format.line = 2;

        TextFormat countdown;
        char countdownText[5];
        sprintf(countdownText, "%d", ((int)_CalibrationTimer / 1000) + 1);

        countdown.horizontalAlignment = TextAlignmentHorizontal::ALIGN_CENTER_HORIZONTAL;
        countdown.verticalAlignment = TextAlignmentVertical::TEXT_LINE;
        countdown.line = 3;

        Display_Utils::printFormattedText(tdd.text.c_str(), tdd.format);
        Display_Utils::printFormattedText(countdownText, countdown);

        Display_Utils::UpdateDisplay().Invoke();
    }

protected:
    int _CalibrationTimer = 0;
    size_t _LastFrameTime = 0;
};