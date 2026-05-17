/**
 * @file SettingsScreen.cpp
 * @brief SettingsScreen implementation.
 */
#include "UI/SettingsScreen.h"
#include "UI/IInputHandler.h"
#include "UI/SDL3InputHandler.h"
#include "UI/SDL3Renderer.h"
#include <array>
#include <string>
#include <vector>

namespace
{
struct ResOption
{
    int w;
    int h;
    const char *label;
};

constexpr std::array<ResOption, 3> kResOptions{{
    {1280, 720, "1280x720  (720p)"},
    {1920, 1080, "1920x1080 (1080p)"},
    {2560, 1440, "2560x1440 (1440p)"},
}};

std::size_t currentResIndex(const Settings &s)
{
    for (std::size_t i{0}; i < kResOptions.size(); ++i)
        if (kResOptions[i].w == s.resolutionWidth &&
            kResOptions[i].h == s.resolutionHeight)
            return i;
    return 0;
}
} // namespace

void SettingsScreen::run(Settings &settings,
                         SDL3Renderer &renderer,
                         SDL3InputHandler &input)
{
    while (true)
    {
        const std::size_t resSel{currentResIndex(settings)};
        const std::vector<std::string> opts{
            "Resolution:    " + std::string{kResOptions[resSel].label} + "  [Enter to cycle]",
            "Master Volume: " + std::to_string(settings.masterVolume) + "  [Enter to +10]",
            "SFX Volume:    " + std::to_string(settings.sfxVolume) + "  [Enter to +10]",
            "Music Volume:  " + std::to_string(settings.musicVolume) + "  [Enter to +10]",
            "<< Back",
        };

        input.setMenuContext("SETTINGS", opts);
        renderer.renderSelectionMenu("SETTINGS", opts);
        const std::size_t pick{input.getMenuChoice(opts.size())};

        if (pick == IInputHandler::kCancelChoice || pick == 4)
        {
            settings.saveToFile("settings.json");
            return;
        }

        if (pick == 0)
        {
            const std::size_t next{(resSel + 1) % kResOptions.size()};
            settings.resolutionWidth = kResOptions[next].w;
            settings.resolutionHeight = kResOptions[next].h;
            renderer.setResolution(settings.resolutionWidth, settings.resolutionHeight);
        }
        else if (pick == 1)
            settings.masterVolume = (settings.masterVolume < 100) ? settings.masterVolume + 10 : 0;
        else if (pick == 2)
            settings.sfxVolume = (settings.sfxVolume < 100) ? settings.sfxVolume + 10 : 0;
        else if (pick == 3)
            settings.musicVolume = (settings.musicVolume < 100) ? settings.musicVolume + 10 : 0;
    }
}