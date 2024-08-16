// #pragma once

// #include "OLED_Content.h"
// #include "globalDefines.h"
// #include "Settings_Manager.h"
// #include "Navigation_Manager.h"
// #include <ArduinoJson.h>
// #include <vector>

// namespace
// {
//     const char *currLocation = "Current Location";
//     const char *noSavedLocations = "No Saved Locations";
//     const char *promptMessage = "Select a location";
// }



// /*
//     * Saved_Locations_Content
//     *
//     * This class is used to display saved locations on the OLED screen.
//     *
//     * The user can scroll through the saved locations and select one.
//     *
//     * If the editMode flag is set to true, the user can add/remove locations.
//     *
//     * Otherwise, a location is selected and passed to the message selector.
//     *
//     * The user can also select the current location if sending a message.

// */

// class Saved_Locations_Content : public OLED_Content
// {
// public:
//     Saved_Locations_Content(bool addCurrentLocation = false, bool promptSelection = false) : showCurrentLocation(addCurrentLocation)
//     {
//         promptSelection = promptSelection;
//         loadLocations();

//         locationIt = savedLocations.begin();
//     }

//     ~Saved_Locations_Content() {}

//     void encUp()
//     {
// #if DEBUG == 1
//         Serial.println("Saved_Locations_Content::encUp()");
// #endif
//         if (locationIt == savedLocations.begin())
//         {
//             locationIt = savedLocations.end();
//             scrollIdx = savedLocations.size();
//         }

//         locationIt--;
//         scrollIdx--;

//         printContent();
//     }

//     void encDown()
//     {
// #if DEBUG == 1
//         Serial.println("Saved_Locations_Content::encDown()");
// #endif
//         locationIt++;
//         scrollIdx++;
//         if (locationIt == savedLocations.end())
//         {
//             locationIt = savedLocations.begin();
//             scrollIdx = 0;
//         }

//         printContent();
//     }

//     void printContent()
//     {
// #if DEBUG == 1
//         Serial.println("Saved_Locations_Content::printContent()");
// #endif
//         Display_Utils::clearContentArea();

//         if (savedLocations.size() == 0)
//         {
// #if DEBUG == 1
//             Serial.println("No saved locations");
// #endif
//             display->setCursor(Display_Utils::centerTextHorizontal(noSavedLocations), Display_Utils::centerTextVertical());
//             display->print(noSavedLocations);
//         }
//         else
//         {
// #if DEBUG == 1
//             Serial.printf("Locations: %d\n", savedLocations.size());
// #endif
//             const char *locationName = locationIt->name;
//             if (!promptSelection)
//             {
// #if DEBUG == 1
//                 Serial.println("Printing a location");
// #endif
//                 // Center location name on the screen
//                 display->setCursor(Display_Utils::centerTextHorizontal(locationName), Display_Utils::centerTextVertical());
//             }
//             else
//             {
// #if DEBUG == 1
//                 Serial.println("Prompting for selection and printing a location");
// #endif
//                 display->setCursor(Display_Utils::centerTextHorizontal(promptMessage), Display_Utils::selectTextLine(2));
//                 display->print(promptMessage);
//                 display->setCursor(Display_Utils::centerTextHorizontal(locationName), Display_Utils::selectTextLine(3));
//                 display->print(locationName);
//             }

//             if (savedLocations.size() > 1)
//             {
//                 LED_Manager::displayScrollWheel(scrollIdx, savedLocations.size());
//             }
//         }

//         display->display();
//     }

//     void deleteEntry()
//     {
//         if (locationIt != savedLocations.end())
//         {
//             Settings_Manager::deleteCoordinate(locationIt->idx);
//             loadLocations();
//             locationIt = savedLocations.begin();
//             Display_Utils::clearContentArea();
//             display->setCursor(Display_Utils::centerTextHorizontal("Location Deleted"), Display_Utils::centerTextVertical());
//             display->print("Location Deleted");
//             display->display();
//             vTaskDelay(2000 / portTICK_PERIOD_MS);
//             printContent();
//         }
//     }

//     Saved_Location *getSelectedLocation()
//     {
//         if (locationIt != savedLocations.end())
//         {
//             return &(*locationIt);
//         }
//         return nullptr;
//     }

//     void loadLocations()
//     {
//         savedLocations.clear();
//         if (showCurrentLocation)
//         {
//             Saved_Location location;
//             location.idx = -1;
//             location.name = currLocation;
//             Navigation_Manager::updateGPS();
//             location.lat = Navigation_Manager::getLocation().lat();
//             location.lon = Navigation_Manager::getLocation().lng();
//             savedLocations.push_back(location);
//         }

//         int locationIdx = 0;
//         for (auto it = Settings_Manager::getCoordIteratorBegin(); it != Settings_Manager::getCoordIteratorEnd(); ++it)
//         {
//             JsonObject coord = it->as<JsonObject>();
//             Saved_Location location;
//             location.idx = locationIdx;
//             location.name = coord["n"].as<const char *>();
//             location.lat = coord["lat"];
//             location.lon = coord["lon"];
//             savedLocations.push_back(location);
//             locationIdx++;
//         }
//     }

// protected:
//     // In select mode, the end iterator is used to display current location
//     std::vector<Saved_Location> savedLocations;
//     std::vector<Saved_Location>::iterator locationIt;
//     size_t scrollIdx = 0;
//     bool showCurrentLocation;
//     bool promptSelection;
// };