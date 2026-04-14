#pragma once
/**
 * @file ITurnOrderCalculator.h
 * @brief Interface for turn order calculation strategies.
 */

/**
 * @brief Calculates the turn order for a round of combat.
 *
 * Injected into Battle via constructor to allow different algorithms
 * (e.g., speed‑based, fixed rotation) and to enable isolated testing.
 */
 
#include "Battle/TurnSlot.h"
#include <vector>

class Party;


class ITurnOrderCalculator
{
  public:
    virtual ~ITurnOrderCalculator() = default;

    /**
     * @brief Build the turn order for the current round.
     * @param playerParty The player's party.
     * @param enemyParty The enemy party.
     * @return A vector of TurnSlot in the order units will act.
     */
    virtual std::vector<TurnSlot> calculate(const Party &playerParty,
                                            const Party &enemyParty) const = 0;
};