#pragma once
/**
 * @file CharacterDetailScreen.h
 * @brief Tab view for a single character: Stats, Equipment, Aspect Tree.
 *        Esc or Back returns to the caller.
 */
#include <string_view>

class MetaProgress;
class CharacterRegistry;
class SDL3Renderer;
class SDL3InputHandler;

class CharacterDetailScreen
{
  public:
    /**
     * @brief Run the character detail screen. Blocks until the player exits.
     * @param characterId Character whose data is displayed.
     * @param meta        Persistent state (level, insight, aspect nodes).
     * @param registry    Constructs the character for before/after stat display.
     * @param renderer    SDL3 renderer.
     * @param input       SDL3 input handler.
     */
    static void run(std::string_view characterId,
                    MetaProgress &meta,
                    const CharacterRegistry &registry,
                    SDL3Renderer &renderer,
                    SDL3InputHandler &input);

  private:
    static void showStatsTab(std::string_view characterId,
                             const MetaProgress &meta,
                             const CharacterRegistry &registry,
                             SDL3Renderer &renderer,
                             SDL3InputHandler &input);

    static void showEquipmentTab(std::string_view characterId,
                                 SDL3Renderer &renderer,
                                 SDL3InputHandler &input);
};