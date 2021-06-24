#include "config.hpp"
#include "beatsaber-hook/shared/config/config-utils.hpp"
#include "modloader/shared/modloader.hpp"

static ModInfo modInfo = {ID, VERSION};
extern Logger& getLogger();

#define INFO(value...) getLogger().info(value)
#define ERROR(value...) getLogger().error(value)

Configuration& getConfig()
{
    static Configuration config(modInfo);
    config.Load();
    return config;
}

config_t config;

void SaveConfig()
{
    INFO("Saving Config...");
    getConfig().config.RemoveAllMembers();
    getConfig().config.SetObject();
    rapidjson::Document::AllocatorType& allocator = getConfig().config.GetAllocator();

    getConfig().config.AddMember("enabled", config.enabled, allocator);
    getConfig().config.AddMember("isTrigger", config.isTrigger, allocator);
    getConfig().config.AddMember("gravityAmount", config.gravityAmount, allocator);
    getConfig().Write();
    INFO("Saved Config!");
}

bool LoadConfig()
{
    INFO("Loading config...");
    bool foundEverything = true;
    rapidjson::Document& doc = getConfig().config;
    if(doc.HasMember("enabled") && doc["enabled"].IsBool()) {
        config.enabled = doc["enabled"].GetBool();
    } else {
        foundEverything = false;
    }
    if(doc.HasMember("isTrigger") && doc["isTrigger"].IsBool()) {
        config.enabled = doc["isTrigger"].GetBool();
    } else {
        foundEverything = false;
    }
    if(doc.HasMember("gravityAmount") && doc["gravityAmount"].IsFloat()) {
        config.enabled = doc["gravityAmount"].GetFloat();
    } else {
        foundEverything = false;
    }
    if (foundEverything) INFO("Config Loaded!");
    return foundEverything;
}