#include <ArduinoJson.h>

namespace FilesystemModule
{
    enum class SettingObjectType 
    {
        ReadOnly = 0,
        Integer,
        Float,
        String,
        Bool,
        Enum
        // add more as needed
    };  

    const std::string SETTINGS_OBJECT_CONFIG_VALUE = "cfgVal";
    const std::string SETTINGS_OBJECT_DEFAULT_VALUE = "dftVal";
    const std::string SETTINGS_OBJECT_CONFIG_TYPE = "cfgType";

    bool canConvertFromJson(JsonVariantConst src, const SettingsObjectInterface&)
    {
        if (!src[SETTINGS_OBJECT_CONFIG_TYPE].isNull() &&
            !src[SETTINGS_OBJECT_CONFIG_VALUE].isNull() &&
            !src[SETTINGS_OBJECT_DEFAULT_VALUE].isNull())
        {
            return true;
        }

        return false;
    }

    class SettingsObjectInterface
    {
    public:
        JsonVariant ConfigValue() = 0;
        void SetConfigValue(JsonVariant value) = 0;

        JsonVariant DefaultValue() = 0;
        void SetDefaultValue(JsonVariant value) = 0;

        SettingObjectType ConfigType() = 0;
    };
}
