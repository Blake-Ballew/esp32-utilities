#include "NavigationUtils.h"

CompassInterface *NavigationUtils::_Compass = nullptr;
TinyGPSPlus NavigationUtils::_GPS;
Stream &NavigationUtils::_GpsInputStream = Serial2;

TinyGPSLocation NavigationUtils::_LastCoordinate;

EventHandler NavigationUtils::_SavedLocationsUpdated;
std::vector<SavedLocation> NavigationUtils::_SavedLocations;

void NavigationUtils::Init(CompassInterface *compass)
{
    _Compass = compass;
}

void NavigationUtils::Init(CompassInterface *compass, Stream &gpsInputStream)
{
    _Compass = compass;
    _GpsInputStream = gpsInputStream;
}

void NavigationUtils::UpdateGPS()
{
    while (_GpsInputStream.available() > 0) {
        _GPS.encode(_GpsInputStream.read());
    }

    if (_GPS.location.isValid())
    {
        _LastCoordinate = _GPS.location;
    }
}

bool NavigationUtils::IsGPSConnected()
{
    return _LastCoordinate.isValid();
}

TinyGPSLocation NavigationUtils::GetLocation()
{
    return _LastCoordinate;
}

TinyGPSTime NavigationUtils::GetTime()
{
    return _GPS.time;
}

TinyGPSDate NavigationUtils::GetDate()
{
    return _GPS.date;
}

uint64_t NavigationUtils::GetTimeDifference(uint32_t time1, uint32_t date1, uint32_t time2, uint32_t date2)
{
#if DEBUG == 1
    // Serial.println("Navigation_Manager::getTimeDifference()");
    // Serial.print("time1: ");
    // Serial.println(time1);
    // Serial.print("date1: ");
    // Serial.println(date1);
    // Serial.print("time2: ");
    // Serial.println(time2);
    // Serial.print("date2: ");
    // Serial.println(date2);
#endif

    uint8_t csec1, sec1, min1, hour1, day1, month1 = 0;
    uint16_t year1 = 0;
    uint8_t csec2, sec2, min2, hour2, day2, month2 = 0;
    uint16_t year2 = 0;

    csec1 = time1 % 100;
    sec1 = (time1 / 100) % 100;
    min1 = (time1 / 10000) % 100;
    hour1 = (time1 / 1000000) % 100;
    day1 = date1 % 100;
    month1 = (date1 / 100) % 100;
    year1 = (date1 / 10000) % 10000;

    csec2 = time2 % 100;
    sec2 = (time2 / 100) % 100;
    min2 = (time2 / 10000) % 100;
    hour2 = (time2 / 1000000) % 100;
    day2 = date2 % 100;
    month2 = (date2 / 100) % 100;
    year2 = (date2 / 10000) % 10000;

    #if DEBUG == 1
    // Serial.print("csec1: ");
    // Serial.println(csec1);
    // Serial.print("sec1: ");
    // Serial.println(sec1);
    // Serial.print("min1: ");
    // Serial.println(min1);
    // Serial.print("hour1: ");
    // Serial.println(hour1);
    // Serial.print("day1: ");
    // Serial.println(day1);
    // Serial.print("month1: ");
    // Serial.println(month1);
    // Serial.print("year1: ");
    // Serial.println(year1);
    // Serial.print("csec2: ");
    // Serial.println(csec2);
    // Serial.print("sec2: ");
    // Serial.println(sec2);
    // Serial.print("min2: ");
    // Serial.println(min2);
    // Serial.print("hour2: ");
    // Serial.println(hour2);
    // Serial.print("day2: ");
    // Serial.println(day2);
    // Serial.print("month2: ");
    // Serial.println(month2);
    // Serial.print("year2: ");
    // Serial.println(year2);
    #endif

    uint64_t diff = 0;

    // Centiseconds
    if (csec1 > csec2)
    {
        csec2 += 100;
        sec2--;
    }

    diff |= (csec2 - csec1);

    // Seconds
    if (sec1 > sec2)
    {
        sec2 += 60;
        min2--;
    }

    diff |= (sec2 - sec1) << 8;

    // Minutes
    if (min1 > min2)
    {
        min2 += 60;
        hour2--;
    }

    diff |= (min2 - min1) << 16;

    // Hours
    if (hour1 > hour2)
    {
        hour2 += 24;
        day2--;
    }

    diff |= (hour2 - hour1) << 24;

    // Days
    if (day1 > day2)
    {
        // Get number of days in month
        uint8_t daysInMonth = 0;
        switch (month2)
        {
        case 1:
        case 3:
        case 5:
        case 7:
        case 8:
        case 10:
        case 12:
            daysInMonth = 31;
            break;
        case 2:
            if (year2 % 4 == 0)
            {
                daysInMonth = 29;
            }
            else
            {
                daysInMonth = 28;
            }
            break;
        case 4:
        case 6:
        case 9:
        case 11:
            daysInMonth = 30;
            break;
        }

        day2 += daysInMonth;
        month2--;
    }

    // Date data stored in upper 32 bits
    diff |= ((uint64_t)(day2 - day1) << 32);

    // Months
    if (month1 > month2)
    {
        month2 += 12;
        year2--;
    }
    diff |= ((uint64_t)(month2 - month1) << 40);

    // Years
    diff |= ((uint64_t)(year2 - year1) << 48);

#if DEBUG == 1
    // Serial.print("diff: ");
    // Serial.println(diff);
#endif

    return diff;    
}

uint64_t NavigationUtils::GetTimeDifference(uint32_t time1, uint32_t date1)
{
    NavigationUtils::UpdateGPS();

    uint32_t time2 = GetTime().value();
    uint32_t date2 = GetDate().value();

    return GetTimeDifference(time1, date1, time2, date2);
}

int NavigationUtils::GetAzimuth()
{
    if (_Compass == nullptr)
    {
        return -1;
    }

    return _Compass->GetAzimuth();
}

double NavigationUtils::GetDistanceTo(double lat, double lon)
{
    UpdateGPS();

    // if (!_LastCoordinate.isValid())
    // {
    //     return -1;
    // }

    #if DEBUG == 1
    Serial.println("GetDistanceTo()");
    Serial.print("My Lat: ");
    Serial.println(_LastCoordinate.lat());
    Serial.print("My Lon: ");
    Serial.println(_LastCoordinate.lng());
    Serial.print("Target Lat: ");
    Serial.println(lat);
    Serial.print("Target Lon: ");
    Serial.println(lon);
    #endif

    return _GPS.distanceBetween(_LastCoordinate.lat(), _LastCoordinate.lng(), lat, lon);
}

double NavigationUtils::GetHeadingTo(double lat, double lon)
{
    UpdateGPS();

    if (!_LastCoordinate.isValid())
    {
        return -1;
    }

    return _GPS.courseTo(_LastCoordinate.lat(), _LastCoordinate.lng(), lat, lon);
}
// Implement later. Was only used for debug screen
int NavigationUtils::GetX()
{
    return 0;
}

int NavigationUtils::GetY()
{
    return 0;
}

int NavigationUtils::GetZ()
{
    return 0;
}

void NavigationUtils::AddSavedLocation(SavedLocation location)
{
    _SavedLocations.push_back(location);

    _SavedLocationsUpdated.Invoke();
}

void NavigationUtils::RemoveSavedLocation(std::vector<SavedLocation>::iterator &locationIt)
{
    locationIt = _SavedLocations.erase(locationIt);

    _SavedLocationsUpdated.Invoke();
}

void NavigationUtils::UpdateSavedLocation(std::vector<SavedLocation>::iterator &locationIt, SavedLocation location)
{
    locationIt->Name = location.Name;
    locationIt->Latitude = location.Latitude;
    locationIt->Longitude = location.Longitude;
}

void NavigationUtils::SerializeSavedLocations(JsonDocument &doc)
{
    JsonArray locationArray;

    if (doc.containsKey("locations"))
    {
        locationArray = doc["locations"].as<JsonArray>();
    }
    else
    {
        locationArray = doc.createNestedArray("locations");
    }

    for (auto location : _SavedLocations)
    {
        JsonObject locationObject = locationArray.createNestedObject();
        locationObject["name"] = location.Name;
        locationObject["lat"] = location.Latitude;
        locationObject["lng"] = location.Longitude;
    }

    #if DEBUG == 1
    Serial.println("Saving location list: ");
    serializeJson(doc, Serial);
    Serial.println();
    #endif
}

void NavigationUtils::DeserializeSavedLocations(JsonDocument &doc)
{
    auto locationArray = doc["locations"].as<JsonArray>();
    _SavedLocations.clear();

    for (auto location : locationArray)
    {
        SavedLocation savedLocation;
        savedLocation.Name = location["name"].as<std::string>();
        savedLocation.Latitude = location["lat"].as<double>();
        savedLocation.Longitude = location["lng"].as<double>();
        _SavedLocations.push_back(savedLocation);
    }
}

void NavigationUtils::FlashSampleLocations()
{
    _SavedLocations.clear();

    SavedLocation nyc;
    nyc.Name = "NYC";
    nyc.Latitude = 40.7128;
    nyc.Longitude = -74.0060;
    _SavedLocations.push_back(nyc);

    SavedLocation sf;
    sf.Name = "SF";
    sf.Latitude = 37.7749;
    sf.Longitude = -122.4194;
    _SavedLocations.push_back(sf);

    SavedLocation atl;
    atl.Name = "ATL";
    atl.Latitude = 33.7490;
    atl.Longitude = -84.3880;
    _SavedLocations.push_back(atl);

    _SavedLocationsUpdated.Invoke();
}
