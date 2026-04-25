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
    ESP_LOGI(TAG, "Updating GPS data...");

    while (_GpsInputStream.available() > 0) {
        _GPS.encode(_GpsInputStream.read());
    }

    if (_GPS.location.isValid())
    {
        ESP_LOGI(TAG, "Valid GPS location received. Latitude: %f, Longitude: %f", _GPS.location.lat(), _GPS.location.lng());
        _LastCoordinate = _GPS.location;
    }
    else
    {
        ESP_LOGW(TAG, "Invalid GPS location. Sattelite count: %d", _GPS.satellites.value());
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
    time_t t1 = PackedToTimeT(time1, date1);
    time_t t2 = PackedToTimeT(time2, date2);
    time_t diffSec = (t2 >= t1) ? (t2 - t1) : 0;

    uint8_t  s  = diffSec % 60; diffSec /= 60;
    uint8_t  mn = diffSec % 60; diffSec /= 60;
    uint8_t  h  = diffSec % 24; diffSec /= 24;
    uint32_t d  = (uint32_t)diffSec;

    uint64_t diff = 0;
    diff |= (uint64_t)s  << 8;
    diff |= (uint64_t)mn << 16;
    diff |= (uint64_t)h  << 24;
    diff |= (uint64_t)(d & 0xFF) << 32;
    return diff;
}

uint64_t NavigationUtils::GetTimeDifference(uint32_t time1, uint32_t date1)
{
    time_t now = 0;
    System_Utils::GetCurrentUTC(now);
    time_t msgTime = PackedToTimeT(time1, date1);
    time_t diffSec = (now >= msgTime) ? (now - msgTime) : 0;

    uint8_t  s  = diffSec % 60; diffSec /= 60;
    uint8_t  mn = diffSec % 60; diffSec /= 60;
    uint8_t  h  = diffSec % 24; diffSec /= 24;
    uint32_t d  = (uint32_t)diffSec;

    uint64_t diff = 0;
    diff |= (uint64_t)s  << 8;
    diff |= (uint64_t)mn << 16;
    diff |= (uint64_t)h  << 24;
    diff |= (uint64_t)(d & 0xFF) << 32;
    return diff;
}

int NavigationUtils::GetAzimuth()
{
    if (_Compass == nullptr)
    {
        return -1;
    }

    return _Compass->GetAzimuth();
}

double NavigationUtils::GetDistance(TinyGPSLocation &loc, double lat, double lon)
{
    return _GPS.distanceBetween(loc.lat(), loc.lng(), lat, lon);
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

double NavigationUtils::GetHeading(TinyGPSLocation &loc, double lat, double lon)
{
    UpdateGPS();

    if (!_LastCoordinate.isValid())
    {
        return -1;
    }
    
    return _GPS.courseTo(loc.lat(), loc.lng(), lat, lon);
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
