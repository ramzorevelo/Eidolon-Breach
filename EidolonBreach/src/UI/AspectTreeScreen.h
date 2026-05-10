#pragma once
/**
 * @file AspectTreeScreen.h
 * @brief SDL3-rendered Aspect Tree screen for one character.
 *
 * Shows all five branches, per-branch signal tallies, node lock/unlock/active
 * states, and current Insight balance. Player navigates by keyboard or mouse
 * and spends Insight to activate unlocked nodes.
 */
#include "Core/BehaviorSignal.h"
#include "Meta/AspectTree.h"
#include <string>
#include <vector>

class SDL3Renderer;
class SDL3InputHandler;
class MetaProgress;
class CharacterRegistry;

class AspectTreeScreen
{
  public:
    /**
     * @brief Run the Aspect Tree screen for `characterId`.
     *        Blocks until the player exits (Esc or Back option).
     *        Writes node activations directly into `meta`.
     * @param characterId  Character whose tree is displayed.
     * @param meta         Persistent state (modified on node activation).
     * @param registry     Used to preview stats of the character at current level.
     * @param renderer     SDL3 renderer.
     * @param input        SDL3 input handler.
     */
    static void run(std::string_view characterId,
                    MetaProgress &meta,
                    const CharacterRegistry &registry,
                    SDL3Renderer &renderer,
                    SDL3InputHandler &input);

  private:
    static void drawTree(const AspectTree &tree,
                         const MetaProgress &meta,
                         std::string_view characterId,
                         std::size_t selectedIndex,
                         SDL3Renderer &renderer);

    static std::vector<std::size_t>
    buildSelectableIndices(const AspectTree &tree,
                           const MetaProgress &meta,
                           std::string_view characterId);
};