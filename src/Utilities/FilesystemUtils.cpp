#include "FilesystemUtils.h"

DynamicJsonDocument FilesystemModule::Utilities::_SettingsFile(4096); 
std::string FilesystemModule::Utilities::_SettingsFilename = "";

EventHandlerT<ArduinoJson::JsonDocument &> FilesystemModule::Utilities::_SettingsUpdated;
