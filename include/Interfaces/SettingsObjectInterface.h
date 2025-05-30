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
        if (src.containsKey(SETTINGS_OBJECT_CONFIG_TYPE) &&
            src.containsKey(SETTINGS_OBJECT_CONFIG_VALUE) &&
            src.containsKey(SETTINGS_OBJECT_DEFAULT_VALUE))
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
