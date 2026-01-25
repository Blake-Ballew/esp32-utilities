#include "NavigationUtils.h"

CompassInterface *NavigationUtils::_Compass = nullptr;
TinyGPSPlus NavigationUtils::_GPS;
Stream &NavigationUtils::_GpsInputStream = Serial2;

TinyGPSLocation NavigationUtils::_LastCoordinate;

EventHandler<> NavigationUtils::_SavedLocationsUpdated;
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
    ESP_LOGD(TAG, "Navigation_Manager::getTimeDifference()");
    ESP_LOGD(TAG, "time1: %lu", time1);
    ESP_LOGD(TAG, "date1: %lu", date1);
    ESP_LOGD(TAG, "time2: %lu", time2);
    ESP_LOGD(TAG, "date2: %lu", date2);

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

    ESP_LOGV(TAG, "csec1: %d", csec1);
    ESP_LOGV(TAG, "sec1: %d", sec1);
    ESP_LOGV(TAG, "min1: %d", min1);
    ESP_LOGV(TAG, "hour1: %d", hour1);
    ESP_LOGV(TAG, "day1: %d", day1);
    ESP_LOGV(TAG, "month1: %d", month1);
    ESP_LOGV(TAG, "year1: %d", year1);
    ESP_LOGV(TAG, "csec2: %d", csec2);
    ESP_LOGV(TAG, "sec2: %d", sec2);
    ESP_LOGV(TAG, "min2: %d", min2);
    ESP_LOGV(TAG, "hour2: %d", hour2);
    ESP_LOGV(TAG, "day2: %d", day2);
    ESP_LOGV(TAG, "month2: %d", month2);
    ESP_LOGV(TAG, "year2: %d", year2);

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

    ESP_LOGV(TAG, "diff: %llu", diff);

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

    ESP_LOGI(TAG, "GetDistanceTo()");
    ESP_LOGI(TAG, "My Lat: %f", _LastCoordinate.lat());
    ESP_LOGI(TAG, "My Lon: %f", _LastCoordinate.lng());
    ESP_LOGI(TAG, "Target Lat: %f", lat);
    ESP_LOGI(TAG, "Target Lon: %f", lon);

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

void NavigationUtils::AddSavedLocation(SavedLocation location, bool updateSavedLocations)
{
    _SavedLocations.push_back(location);

    if (updateSavedLocations)
        _SavedLocationsUpdated.Invoke();
}

void NavigationUtils::RemoveSavedLocation(std::vector<SavedLocation>::iterator &locationIt)
{
    locationIt = _SavedLocations.erase(locationIt);

    _SavedLocationsUpdated.Invoke();
}

void NavigationUtils::ClearSavedLocations()
{
    _SavedLocations.clear();
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

    if (doc.containsKey("Locations"))
    {
        locationArray = doc["Locations"].as<JsonArray>();
    }
    else
    {
        locationArray = doc.createNestedArray("Locations");
    }

    for (auto location : _SavedLocations)
    {
        JsonObject locationObject = locationArray.createNestedObject();
        locationObject["Name"] = location.Name;
        locationObject["Lat"] = location.Latitude;
        locationObject["Lng"] = location.Longitude;
    }

    std::string buf;
    serializeJson(doc, buf);
    ESP_LOGV(TAG, "Saving location list: %s", buf.c_str());
}

void NavigationUtils::DeserializeSavedLocations(JsonDocument &doc)
{
    auto locationArray = doc["Locations"].as<JsonArray>();
    _SavedLocations.clear();

    for (auto location : locationArray)
    {
        SavedLocation savedLocation;
        savedLocation.Name = location["Name"].as<std::string>();
        savedLocation.Latitude = location["Lat"].as<double>();
        savedLocation.Longitude = location["Lng"].as<double>();
        _SavedLocations.push_back(savedLocation);
    }
}

void NavigationUtils::RpcAddSavedLocation(JsonDocument &doc)
{
    if (doc.containsKey("Name") && doc.containsKey("Lat") && doc.containsKey("Lng"))
    {
        SavedLocation location;
        location.Name = doc["Name"].as<std::string>();
        location.Latitude = doc["Lat"].as<double>();
        location.Longitude = doc["Lng"].as<double>();
        AddSavedLocation(location, true);
    }

    doc.clear();
}

void NavigationUtils::RpcAddSavedLocations(JsonDocument &doc)
{
    if (doc.containsKey("Locations"))
    {
        auto locations = doc["Locations"].as<JsonArray>();
        for (auto location : locations)
        {
            SavedLocation savedLocation;
            savedLocation.Name = location["Name"].as<std::string>();
            savedLocation.Latitude = location["Lat"].as<double>();
            savedLocation.Longitude = location["Lng"].as<double>();
            AddSavedLocation(savedLocation, false);
        }

        SavedLocationsUpdated().Invoke();
    }

    doc.clear();
}

void NavigationUtils::RpcRemoveSavedLocation(JsonDocument &doc)
{
    bool success = false;

    if (doc.containsKey("Idx"))
    {
        auto idx = doc["Idx"].as<int>();
        if (idx >= 0 && idx < _SavedLocations.size())
        {
            success = true;
            auto locationIt = _SavedLocations.begin() + idx;
            RemoveSavedLocation(locationIt);
        }
    }

    doc.clear();

    doc["Success"] = success;
}

void NavigationUtils::RpcClearSavedLocations(JsonDocument &doc)
{
    ClearSavedLocations();
    doc.clear();
}

void NavigationUtils::RpcUpdateSavedLocation(JsonDocument &doc)
{
    bool success = false;
    if (doc.containsKey("Idx") && doc.containsKey("Name") && doc.containsKey("Lat") && doc.containsKey("Lng"))
    {
        auto idx = doc["Idx"].as<int>();
        if (idx >= 0 && idx < _SavedLocations.size())
        {
            success = true;
            auto locationIt = _SavedLocations.begin() + idx;
            UpdateSavedLocation(locationIt, {doc["Name"].as<std::string>(), doc["Lat"].as<double>(), doc["Lng"].as<double>()});
        }
    }

    doc.clear();

    doc["Success"] = success;
}

void NavigationUtils::RpcGetSavedLocation(JsonDocument &doc)
{
    if (doc.containsKey("Idx"))
    {
        auto idx = doc["Idx"].as<int>();
        doc.clear();
        if (idx >= 0 && idx < _SavedLocations.size())
        {
            auto locationIt = _SavedLocations.begin() + idx;
            doc["Name"] = locationIt->Name;
            doc["Lat"] = locationIt->Latitude;
            doc["Lng"] = locationIt->Longitude;
        }
    }
}

void NavigationUtils::RpcGetSavedLocations(JsonDocument &doc)
{
    doc.clear();
    JsonArray locationArray = doc.createNestedArray("Locations");
    for (auto location : _SavedLocations)
    {
        JsonObject locationObject = locationArray.createNestedObject();
        locationObject["Name"] = location.Name;
        locationObject["Lat"] = location.Latitude;
        locationObject["Lng"] = location.Longitude;
    }

    std::string buf;
    serializeJsonPretty(doc, buf);
    ESP_LOGV(TAG, "%s", buf.c_str());
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
