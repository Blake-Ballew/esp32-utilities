#include "FilesystemUtils.h"

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
    File file = SPIFFS.open(filename.c_str(), FILE_READ);

    if (!file)
    {
        return FilesystemReturnCode::FILE_NOT_FOUND;
    }

    deserializeMsgPack(doc, file); 
    file.close();

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