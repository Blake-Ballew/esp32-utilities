#include "RadioUtils.h"

namespace ConnectivityModule
{


    class EspNowManager
    {
    public:
        EspNowManager() {}
        ~EspNowManager() {}

        // Radio channel to use for ESP-NOW
        void int &RadioChannel()
        {
            static int channel = 1;
            return channel;
        }
    }
}