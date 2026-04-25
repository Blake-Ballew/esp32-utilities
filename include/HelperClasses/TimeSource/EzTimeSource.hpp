#pragma once
#include "TimeSourceInterface.hpp"
#include <ezTime.h>

class EzTimeSource : public SystemModule::TimeSourceInterface
{
public:
    bool TryGetCurrentUTC(time_t& outTime) override
    {
        if (timeStatus() == timeNotSet) 
        { 
            ESP_LOGI(_TAG, "Failed to get Ez time");
            return false; 
        }
        outTime = UTC.now();
        ESP_LOGI(_TAG, "Returning timestamp %d from EzTime dead reckoning", outTime);
        return true;
    }

private:
    const char * _TAG = "EzTimeSource";
};
