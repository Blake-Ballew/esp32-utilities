#include "OLED_Window.h"
#include "NavigationUtils.h"

#include "RingPoint.h"
#include "LED_Utils.h"

namespace
{
    const size_t GPS_WINDOW_REFRESH_RATE_MS = 1000;
}

class GPS_Window : public OLED_Window
{
public:
    GPS_Window(OLED_Window *parent) : OLED_Window(parent)
    {
        state.assignInput(BUTTON_3, ACTION_BACK, "Back");
        Display_Utils::enableRefreshTimer(GPS_WINDOW_REFRESH_RATE_MS);
        currentState = &state;
    }

    ~GPS_Window() 
    {
        Display_Utils::disableRefreshTimer();
    }

    void Pause()
    {
        Display_Utils::disableRefreshTimer();
    }

    void Resume()
    {
        Display_Utils::enableRefreshTimer(GPS_WINDOW_REFRESH_RATE_MS);
    }

    void drawWindow()
    {
        OLED_Window::drawWindow();

        if (NavigationUtils::IsGPSConnected())
        {
            NavigationUtils::UpdateGPS();
            auto location = NavigationUtils::GetLocation();

            Display_Utils::getDisplay()->setCursor(Display_Utils::alignTextLeft(), Display_Utils::selectTextLine(1));
            Display_Utils::getDisplay()->print("Lat: ");
            Display_Utils::getDisplay()->print(location.lat(), 8);

            Display_Utils::getDisplay()->setCursor(Display_Utils::alignTextLeft(), Display_Utils::selectTextLine(2));
            Display_Utils::getDisplay()->print("Lon: ");
            Display_Utils::getDisplay()->print(location.lng(), 8);
        }
        else
        {
            Display_Utils::printCenteredText("GPS Not Connected");
        }

        Display_Utils::UpdateDisplay().Invoke();
    }

protected:
    Window_State state;
};