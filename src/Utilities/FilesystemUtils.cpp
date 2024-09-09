#include "FilesystemUtils.h"

DynamicJsonDocument FilesystemUtils::_SettingsFile(2048); 
std::string FilesystemUtils::_SettingsFilename = "";

EventHandler FilesystemUtils::_SettingsUpdated;

void FilesystemUtils::Init()
{
    if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED))
    {
        Serial.println("SPIFFS Mount Failed. Rebooting...");
        ESP.restart();
    }
}

FilesystemReturnCode FilesystemUtils::ReadFile(std::string filename, JsonDocument &doc)
{   
    if (!SPIFFS.exists(filename.c_str()))
    {
        return FilesystemReturnCode::FILE_NOT_FOUND;
    }

    File file = SPIFFS.open(filename.c_str(), FILE_READ);

    if (!file)
    {
        return FilesystemReturnCode::READ_ERROR;
    }

    deserializeMsgPack(doc, file); 
    file.close();

    if (doc.overflowed())
    {
        return FilesystemReturnCode::READ_BUFFER_OVERFLOW;
    }

    return FilesystemReturnCode::FILESYSTEM_OK;
}

FilesystemReturnCode FilesystemUtils::WriteFile(std::string filename, JsonDocument &doc)
{
    File file = SPIFFS.open(filename.c_str(), FILE_WRITE);
    if (!file)
    {
        return FilesystemReturnCode::WRITE_FAILED;
    }

    auto bytesWritten = serializeMsgPack(doc, file);
    file.close();

    if (bytesWritten == 0)
    {
        return FilesystemReturnCode::WRITE_FAILED;
    }

    return FilesystemReturnCode::FILESYSTEM_OK;
}

FilesystemReturnCode FilesystemUtils::LoadSettingsFile(std::string filename)
{
    _SettingsFile.clear();
    auto returncode = ReadFile(filename, _SettingsFile);

    if (returncode == FilesystemReturnCode::FILESYSTEM_OK)
    {
        _SettingsFilename = filename;
    }

    return returncode;
}