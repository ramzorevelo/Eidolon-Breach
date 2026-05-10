#pragma once
/**
 * @file ITurnOrderCalculator.h
 * @brief Interface for turn order calculation strategies.
 *
 * Injected into Battle to allow different algorithms and isolated testing.
 * applyHasten, applySuppress, and onUnitActed have default no-op
 * implementations so speed-based and test calculators compile unchanged.
 */

#include "Battle/TurnSlot.h"
#include <vector>

class Unit;
class Party;

class ITurnOrderCalculator
{
  public:
    virtual ~ITurnOrderCalculator() = default;

    /**
     * @brief Project the next N turn slots by simulating AV forward.
     *        Soonest-acting unit is first. Capped at kAvProjectionSlots.
     */
    [[nodiscard]] virtual std::vector<TurnSlot> calculate(Party &playerParty,
                                                          Party &enemyParty) const = 0;

    /**
     * @brief Reduce a unit's current AV by pct of its base AV (act sooner).
     *        Clamped so current AV never goes below 0.
     * @param unit Target unit. No-op if unit is not tracked.
     * @param pct  Fraction of base AV to subtract (e.g. 0.15f = 15%).
     */
    virtual void applyHasten(Unit * /*unit*/, float /*pct*/) {}

    /**
     * @brief Increase a unit's current AV by pct of its base AV (act later).
     * @param unit Target unit. No-op if unit is not tracked.
     * @param pct  Fraction of base AV to add (e.g. 1.0f ≈ one full turn).
     */
    virtual void applySuppress(Unit * /*unit*/, float /*pct*/) {}

    /**
     * @brief Called by Battle after a unit's turn slot is consumed.
     *        AV-based calculators use this to advance live AV state.
     *        Default: no-op (round-based calculators ignore it).
     */
    virtual void onUnitActed(Unit * /*unit*/) {}
};