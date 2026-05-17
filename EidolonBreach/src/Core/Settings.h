#pragma once
/**
 * @file Settings.h
 * @brief User preference settings persisted to settings.json.
 *        Resolution drives SDL_SetWindowSize on session start.
 *        Volume fields are stubs; full audio integration is deferred.
 */
#include <filesystem>

struct Settings
{
    int resolutionWidth{1280};
    int resolutionHeight{720};
    int masterVolume{100}; ///< 0–100, inclusive.
    int sfxVolume{100};
    int musicVolume{100};

    /**
     * @brief Load settings from file. Returns defaults if absent or unparseable.
     * @param path File path (e.g. "settings.json").
     */
    [[nodiscard]] static Settings loadFromFile(const std::filesystem::path &path);

    /**
     * @brief Persist settings to file.
     * @param path File path (e.g. "settings.json").
     */
    void saveToFile(const std::filesystem::path &path) const;
};