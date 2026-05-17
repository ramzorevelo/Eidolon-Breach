/**
 * @file Settings.cpp
 * @brief Settings save/load via nlohmann/json.
 */
#include "Core/Settings.h"
#include "nlohmann/json.hpp"
#include <fstream>

Settings Settings::loadFromFile(const std::filesystem::path &path)
{
    Settings s{};
    std::ifstream file{path};
    if (!file.is_open())
        return s;
    nlohmann::json j{};
    try
    {
        file >> j;
    }
    catch (const nlohmann::json::parse_error &)
    {
        return s;
    }
    s.resolutionWidth = j.value("resolutionWidth", 1280);
    s.resolutionHeight = j.value("resolutionHeight", 720);
    s.masterVolume = j.value("masterVolume", 100);
    s.sfxVolume = j.value("sfxVolume", 100);
    s.musicVolume = j.value("musicVolume", 100);
    return s;
}

void Settings::saveToFile(const std::filesystem::path &path) const
{
    nlohmann::json j{};
    j["resolutionWidth"] = resolutionWidth;
    j["resolutionHeight"] = resolutionHeight;
    j["masterVolume"] = masterVolume;
    j["sfxVolume"] = sfxVolume;
    j["musicVolume"] = musicVolume;
    std::ofstream file{path};
    if (file.is_open())
        file << j.dump(2);
}