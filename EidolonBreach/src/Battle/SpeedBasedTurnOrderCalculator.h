#pragma once
/**
 * @file SpeedBasedTurnOrderCalculator.h
 * @brief Default turn order calculator using SPD stat.
 */

/**
 * @brief Calculates turn order by sorting units by SPD (descending).
 *
 * Tie‑breakers (in order):
 * - Player units act before enemy units.
 * - Within the same party, lower formation index acts first.
 */

#include "Battle/ITurnOrderCalculator.h"
#include <vector>

class SpeedBasedTurnOrderCalculator : public ITurnOrderCalculator
{
  public:
    std::vector<TurnSlot> calculate(const Party &playerParty,
                                    const Party &enemyParty) const override;
};