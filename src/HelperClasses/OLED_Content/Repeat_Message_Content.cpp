// #include "Repeat_Message_Content.h"

// size_t Repeat_Message_Content::currentTick = 0;

// Repeat_Message_Content::Repeat_Message_Content(bool newMsgID)
// {
// }

// Repeat_Message_Content::~Repeat_Message_Content()
// {
// }

// void Repeat_Message_Content::printContent()
// {
// #if DEBUG == 1
//     // Serial.println("Repeat_Message_Content::printContent()");
//     // Serial.printf("Repeat_Message_Content::printContent(): confirmed: %d\n", confirmed);
//     // Serial.printf("Repeat_Message_Content::printContent(): currentTick: %d\n", currentTick);
// #endif
//     Display_Utils::clearContentArea();
//     if (!confirmed)
//     {
//         Display_Utils::clearContentArea();
//         display->setCursor(Display_Utils::centerTextHorizontal("Send SOS?"), Display_Utils::centerTextVertical());
//         display->print("Send SOS?");
//         display->display();
//         return;
//     }
//     else
//     {
//         display->setCursor(Display_Utils::centerTextHorizontal(14), Display_Utils::centerTextVertical());
//         display->print("Sending SOS");
//         for (int i = 0; i < ((currentTick / 60) % 3) + 1; i++)
//         {
//             display->print(".");
//         }
//         LED_Manager::pulseCircle(255, 0, 0, currentTick);
//         display->display();
//     }
// }

// void Repeat_Message_Content::Pause()
// {
//     Display_Utils::enableRefreshTimer();
// }

// void Repeat_Message_Content::Resume()
// {
//     if (thisInstance == nullptr)
//     {
//         return;
//     }
//     xTimerStart(updateTimer, 0);
// }

// void Repeat_Message_Content::confirmSOS()
// {
//     confirmed = true;
//     thisInstance = this;
//     currentTick = Repeat_Message_Content_TICKS_PER_MESSAGE;
//     xTimerStart(updateTimer, 0);
//     msgID = esp_random();
// }

// void Repeat_Message_Content::unconfirmSOS()
// {
//     confirmed = false;
//     thisInstance = nullptr;
//     xTimerStop(updateTimer, 1000);
//     sendOkay();
// }

// void Repeat_Message_Content::updateDisplay(TimerHandle_t timer)
// {
//     if (thisInstance == nullptr)
//     {
//         xTimerStop(timer, 0);
//         return;
//     }

//     thisInstance->printContent();
//     currentTick++;
//     if (currentTick >= Repeat_Message_Content_TICKS_PER_MESSAGE)
//     {
//         thisInstance->sendSOS();
//         currentTick = 0;
//     }
// }

// void Repeat_Message_Content::sendSOS()
// {
//     if (!confirmed)
//     {
//         return;
//     }

//     msgID = esp_random();

//     Navigation_Manager::read();

//     uint32_t time, date;
//     time = Navigation_Manager::getTime().value();
//     date = Navigation_Manager::getDate().value();

//     uint64_t sender = Network_Manager::userID;
//     const char *senderName = Settings_Manager::settings["User"]["Name"]["cfgVal"].as<const char *>();

//     uint8_t R = 255;
//     uint8_t G = 0;
//     uint8_t B = 0;

//     TinyGPSLocation coords = Navigation_Manager::getLocation();

// #if USE_FAKE_GPS_COORDS == 0
//     double lat = coords.lat();
//     double lng = coords.lng();
// #else
//     double lat = FAKE_GPS_LAT;
//     double lng = FAKE_GPS_LON;
// #endif

//     const char *status = Repeat_Message_Content_MESSAGE;
//     Message_Ping *msgPing = new Message_Ping(time, date, 0, sender, senderName, msgID, R, G, B, lat, lng, status);

//     uint8_t returnCode = Network_Manager::queueMessage(msgPing);
// }

// void Repeat_Message_Content::sendOkay()
// {
//     if (confirmed)
//     {
//         return;
//     }

//     msgID = esp_random();

//     Navigation_Manager::read();

//     uint32_t time, date;
//     time = Navigation_Manager::getTime().value();
//     date = Navigation_Manager::getDate().value();

//     uint64_t sender = Network_Manager::userID;
//     const char *senderName = Settings_Manager::settings["User"]["Name"]["cfgVal"].as<const char *>();

//     uint8_t R = 0;
//     uint8_t G = 255;
//     uint8_t B = 0;

//     TinyGPSLocation coords = Navigation_Manager::getLocation();

// #if USE_FAKE_GPS_COORDS == 0
//     double lat = coords.lat();
//     double lng = coords.lng();
// #else
//     double lat = FAKE_GPS_LAT;
//     double lng = FAKE_GPS_LON;
// #endif

//     const char *status = OK_CONTENT_MESSAGE;
//     Message_Ping *msgPing = new Message_Ping(time, date, 0, sender, senderName, msgID, R, G, B, lat, lng, status);

//     uint8_t returnCode = Network_Manager::queueMessage(msgPing);
// }