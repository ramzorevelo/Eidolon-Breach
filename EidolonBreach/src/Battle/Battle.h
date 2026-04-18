#pragma once
/**
 * @file Battle.h
 * @brief Orchestrates turn‑based combat between two Parties.
 */

#include "Battle/ITurnOrderCalculator.h"
#include "Battle/TurnSlot.h"
#include "Entities/Party.h"
#include "UI/ConsoleRenderer.h"
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
    void renderNewBreaks(const std::vector<bool> &before, const Party &party) const;
    bool isBattleOver() const;
    void processPlayerTurn(Unit *unit);
    void processEnemyTurn(Unit *unit);
    bool checkAndHandleBattleEnd();
    mutable ConsoleRenderer m_renderer;
    ResonanceField m_field{};
    void processPlayerTurn(Unit *unit, BattleState &state);
    void processEnemyTurn(Unit *unit, BattleState &state);
};