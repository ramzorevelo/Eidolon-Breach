#pragma once
/**
 * @file HubScreen.h
 * @brief Main hub. Owns the top-level event loop.
 *        Replaces the linear renderSelectionMenu loop in main().
 *        QuitException from any nested screen propagates to main() unchanged.
 */
#include "Core/Settings.h"

class SDL3Renderer;
class SDL3InputHandler;
class CharacterRegistry;
class AbilityRegistry;
class SummonRegistry;
class MetaProgress;

class HubScreen
{
  public:
    /**
     * @brief Construct the hub with all dependencies.
     * @param renderer          SDL3 renderer (owned by main).
     * @param input             SDL3 input handler (owned by main).
     * @param characterRegistry Character registry.
     * @param abilityRegistry   Ability registry (forwarded to BondTrial::run).
     * @param summonRegistry    Summon registry.
     * @param meta              Persistent meta-progress (modified on run completion).
     * @param settings          User settings (modified by SettingsScreen).
     */
    HubScreen(SDL3Renderer &renderer,
              SDL3InputHandler &input,
              CharacterRegistry &characterRegistry,
              const AbilityRegistry &abilityRegistry,
              SummonRegistry &summonRegistry,
              MetaProgress &meta,
              Settings &settings);

    /**
     * @brief Run the hub loop. Returns only when QuitException is thrown.
     */
    void run();

  private:
    void showCharacters();
    void showBackpack();
    void launchClassic();
    void launchLabyrinth();
    void showBondTrials();
    void showSettings();
    void exitGame();

    SDL3Renderer &m_renderer;
    SDL3InputHandler &m_input;
    CharacterRegistry &m_characterRegistry;
    const AbilityRegistry &m_abilityRegistry;
    SummonRegistry &m_summonRegistry;
    MetaProgress &m_meta;
    Settings &m_settings;
};