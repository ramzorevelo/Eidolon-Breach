#pragma once
/**
 * @file IRenderer.h
 * @brief Abstract rendering interface for all game output
 *
 * Battle and other game systems receive IRenderer& at construction.
 * They never instantiate a concrete renderer.
 */
#include "Entities/PlayableCharacter.h" 
#include "Battle/TurnSlot.h"
#include "Core/ActionResult.h"
#include "Core/Drop.h"
#include <optional>
#include <vector>
#include <string>

class Party;
class Unit;
class ResonanceField;

/**
 * @brief Data required to render one dungeon row and its detail panel.
 *        Populated by main.cpp from DungeonDefinition; the renderer does not
 *        depend on DungeonDefinition directly.
 */
struct DungeonSelectInfo
{
    std::string name{};
    std::string description{};
    int recommendedLevel{1};
    int enemyLevel{1};
    /**
     * @brief Ordered node types for the floor layout strip.
     *        Valid strings: "battle", "elite", "boss", "rest", "treasure", "shop".
     */
    std::vector<std::string> layout{};
    bool cleared{false};
    std::string difficultyLabel{}; // "Normal", "Hard", or "Nightmare"
};
class IRenderer
{
  public:
    virtual ~IRenderer() = default;

    /** @brief Render the outcome of an action (damage dealt, heal, skip, etc.). */
    virtual void renderActionResult(const std::string &actorName,
                                    const ActionResult &result) = 0;

    /** @brief Render the break notification for an enemy that just broke. */
    virtual void renderBreak(const std::string &enemyName) = 0;

    /** @brief Render the stun message for a broken enemy skipping its turn. */
    virtual void renderStunned(const std::string &enemyName) = 0;

    /** @brief Render the victory notification and any loot dropped. */
    virtual void renderVictory(const std::string &enemyName,
                               std::optional<Drop> drop) = 0;

    /** @brief Render the defeat notification for a fallen player unit. */
    virtual void renderDefeat(const std::string &playerName) = 0;

    /** @brief Render the full party status display (HP, toughness, effects). */
    virtual void renderPartyStatus(const Party &playerParty,
                                   const Party &enemyParty) = 0;

    /**
     * @brief Render a free-form message (tick messages, resonance field events, etc
     */
    virtual void renderMessage(const std::string &message) = 0;

    /**
     * @brief Render the Resonance Field gauge and vote summary.
     * @param field The current ResonanceField state.
     */
    virtual void renderResonanceField(const ResonanceField &field) = 0;
    /**
     * @brief Render the action menu for the active player character.
     * @param character The acting character (for name, 
     , SP).
     * @param party     The player party (for shared SP display).
     */
    virtual void renderActionMenu(const PlayableCharacter &character,
                                  const Party &party) = 0;
   
    /**
     * @brief Render the upcoming turn order at the start of each round.
     * @param order Slots in the order units will act this round.
     */
    virtual void renderTurnOrder(const std::vector<TurnSlot> &order) = 0;
    /**
     * @brief Render the context-sensitive key binding hint at the bottom of the screen.
     * @param hint One-line string describing current valid inputs.
     */
    virtual void renderHintBar(const std::string &hint) = 0;

    virtual void clearTargetHighlight() {} // default no-op 

    virtual void renderTargetList(const std::vector<std::string> &names,
                                  bool isAllyTarget = false) = 0;
    virtual void updateTargetHighlight(int index)
    {
        (void)index;
    }

    virtual void presentPause(int ms)
    {
        (void)ms;
    }

    /**
     * @brief Render a full-screen numbered selection menu.
     *        Called before getMenuChoice blocks for input.
     * @param title    Header line (e.g. "REST SITE").
     * @param options  List of selectable option strings.
     * @param selected Currently highlighted 0-based index.
     */
    virtual void renderSelectionMenu(const std::string &title,
                                     const std::vector<std::string> &options,
                                     std::size_t selected = 0)
    {
        (void)title;
        (void)options;
        (void)selected;
    }

    virtual void clearBattleCache() {}

    /** @brief Let the renderer play any queued visual effects (damage numbers, break flash, etc.). */
    virtual void flushVisualEffects() {}

    /**
     * @brief Render the dungeon selection screen with a split list/detail layout.
     *        Caches the dungeon infos so that subsequent renderSelectionMenu calls
     *        from SDL3InputHandler (Up/Down navigation) re-draw the split layout.
     *        Default delegates to renderSelectionMenu for NullRenderer compatibility.
     * @param title    Screen title (e.g. "DUNGEON SELECT  Player Lv.5").
     * @param dungeons Ordered list of available dungeon descriptors.
     * @param selected Currently highlighted 0-based index.
     */
    virtual void renderDungeonSelect(const std::string &title,
                                     const std::vector<DungeonSelectInfo> &dungeons,
                                     std::size_t selected = 0)
    {
        std::vector<std::string> opts;
        opts.reserve(dungeons.size());
        for (const auto &d : dungeons)
            opts.push_back(d.name + (d.cleared ? "  [CLEARED]" : ""));
        opts.push_back("<< Back");
        renderSelectionMenu(title, opts, selected);
    }

};

