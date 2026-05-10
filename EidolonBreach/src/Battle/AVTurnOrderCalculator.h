#pragma once
/**
 * @file AVTurnOrderCalculator.h
 * @brief AV-based turn order calculator.
 *
 * Maintains a live AV state map for all units in the current battle.
 * calculate() projects forward by simulating the countdown; onUnitActed()
 * advances the live state after a slot is consumed by Battle.
 * applyHasten and applySuppress are the only external write paths into AV.
 */

#include "Battle/ITurnOrderCalculator.h"
#include <unordered_map>

class AVTurnOrderCalculator : public ITurnOrderCalculator
{
  public:
    [[nodiscard]] std::vector<TurnSlot> calculate(Party &playerParty,
                                                  Party &enemyParty) const override;

    void applyHasten(Unit *unit, float pct) override;
    void applySuppress(Unit *unit, float pct) override;
    void onUnitActed(Unit *unit) override;

  private:
    // Mutable: syncUnits() is logically a cache fill, not observable mutation.
    mutable std::unordered_map<Unit *, float> m_currentAV{};
    mutable std::unordered_map<Unit *, float> m_baseAV{};

    static constexpr int kProjectionSlots{10};

    /** Registers newly seen alive units; does not remove dead ones. */
    void syncUnits(Party &playerParty, Party &enemyParty) const;

    /** Base AV for a unit: kBaseAG / SPD. */
    [[nodiscard]] static float computeBaseAV(const Unit *unit);

    /**
     * @brief Exposure modifier for AV reset. Only PlayableCharacter returns
     *        a value other than 1.0f; all other units return kAvModBaseline.
     */
    [[nodiscard]] static float exposureModifier(Unit *unit);
};