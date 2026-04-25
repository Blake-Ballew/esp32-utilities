#pragma once
#include <Arduino.h>

namespace SystemModule
{
    class TimeSourceInterface
    {
    public:
        // Returns true and populates outTime (UTC, Unix epoch) on success
        virtual bool TryGetCurrentUTC(time_t& outTime) = 0;
        virtual ~TimeSourceInterface() = default;
    };
}
