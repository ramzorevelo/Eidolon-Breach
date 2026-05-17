#pragma once
/**
 * @file SettingsScreen.h
 * @brief SDL3-rendered settings screen: resolution picker and volume stubs.
 *        Resolution changes apply immediately via SDL3Renderer::setResolution.
 *        Volume is persisted but not wired to audio until v0.9.8.
 */
#include "Core/Settings.h"

class SDL3Renderer;
class SDL3InputHandler;

class SettingsScreen
{
  public:
    /**
     * @brief Run the settings screen. Blocks until the player exits (Esc or Back).
     *        Writes settings.json on exit.
     * @param settings Active settings (modified in place).
     * @param renderer SDL3 renderer (receives resolution changes immediately).
     * @param input    SDL3 input handler.
     */
    static void run(Settings &settings,
                    SDL3Renderer &renderer,
                    SDL3InputHandler &input);
};