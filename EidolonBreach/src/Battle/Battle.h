#pragma once
/**
 * @file Battle.h
 * @brief Orchestrates turn‑based combat between two Parties.
 */

#include "Battle/ITurnOrderCalculator.h"
#include "Battle/TurnSlot.h"
#include "Entities/Party.h"
#include "UI/IRenderer.h"
#include "UI/IInputHandler.h"
#include "Battle/BattleState.h"
#include "Battle/ResonanceField.h"
#include <memory>
#include <vector>

class Battle
{
  public:
    /**
     * @brief Construct a battle.
     * @param playerParty Reference to the player's party.
     * @param enemyParty Reference to the enemy party.
     * @param turnOrderCalc Strategy for determining turn order.
     *        If nullptr, defaults to SpeedBasedTurnOrderCalculator.
     */
    Battle(Party &playerParty,
           Party &enemyParty,
           IRenderer &renderer,
           IInputHandler &inputHandler,
           std::unique_ptr<ITurnOrderCalculator> turnOrderCalc = nullptr);

    void run();

    // Expose the calculator for testing (optional; tests can also inject a mock)
    const ITurnOrderCalculator *getTurnOrderCalculator() const
    {
        return m_turnOrderCalc.get();
    }

  private:
    Party &m_playerParty;
    Party &m_enemyParty;
    std::unique_ptr<ITurnOrderCalculator> m_turnOrderCalc;

    std::vector<bool> snapshotBreakStates(const Party &party) const;
    /**
     * @brief Renders break notifications and fires BreakEffect callbacks for
     *        any enemy that transitioned from not-broken to broken this action.
     */
    void processNewBreaks(const std::vector<bool> &before,
                          const Party &party,
                          BattleState &state);
    bool isBattleOver() const;
    bool checkAndHandleBattleEnd(BattleState &state);
    ResonanceField m_field{};
    void applyResonanceTrigger(Affinity affinity);
    void processPlayerTurn(Unit *unit, BattleState &state);
    void processEnemyTurn(Unit *unit, BattleState &state);
    IRenderer &m_renderer;
    IInputHandler &m_inputHandler;
    void resetAllPcConsumableState();
    /**
     * @brief Applies spGained and exposureDelta from an ActionResult.
     * Called after every player action, after vestige onAction hooks (Phase 7).
     * @param actor  The PlayableCharacter who performed the action.
     * @param result The result returned by IAction::execute().
     */
    void processActionResult(PlayableCharacter &actor,
                             Party &allies,
                             const ActionResult &result);
    /**
     * @brief Collect drops from all defeated enemies and add them to the player
     *        party's inventory. Called once at the end of a victorious battle.
     */
    void collectDrops(BattleState &state);
};