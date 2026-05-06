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
     * @brief Render the numbered list of valid targets before the input prompt.
     * @param names Display name of each selectable target, in selection order.
     */
    virtual void renderTargetList(const std::vector<std::string> &names) = 0;
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
};