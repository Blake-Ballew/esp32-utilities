#include "FilesystemUtils.h"

JsonDocument FilesystemModule::Utilities::_SettingsFile; 
std::string FilesystemModule::Utilities::_SettingsFilename = "";

EventHandler<ArduinoJson::JsonDocument &> FilesystemModule::Utilities::_SettingsUpdated;
