#pragma once
/**
 * @file FormationManager.h
 * @brief Manages the 5-slot linear formation for one side of a battle.
 *        Party owns unit lifetimes; FormationManager holds non-owning pointers.
 *        Push/pull overflow damage is computed here.
 */

#include "Core/CombatConstants.h"
#include "Core/CombatUtils.h"
#include "Core/ScalingStat.h"
#include "Core/Stats.h"
#include <array>
#include <cstddef>

class Unit;

class FormationManager
{
  public:
    static constexpr int kMaxSlots{CombatConstants::kMaxFormationSlots};

    /**
     * @brief Insert unit at position. Shifts occupants at and after position
     *        one slot toward the rear. Returns false if the formation is full
     *        or position is out of range.
     */
    bool insert(Unit *unit, int position);

    /**
     * @brief Remove unit and shift all units after it one slot toward position 0.
     *        No-op if unit is not found.
     */
    void remove(Unit *unit);

    /**
     * @brief Swap the positions of unit and the occupant at targetPosition.
     *        If targetPosition is empty, the unit simply moves there.
     *        Never causes overflow damage. Returns false if unit not found.
     */
    bool swap(Unit *unit, int targetPosition);

    /**
     * @brief Apply a push (positive delta = toward rear, negative = toward front)
     *        to unit. Clamps to boundaries. Returns the overflow damage amount
     *        (> 0 only when the push was blocked by a boundary).
     *        The caller is responsible for calling unit->takeTrueDamage(overflow).
     *
     * @param unit           Unit being pushed.
     * @param delta          Signed slot displacement.
     * @param skillPower     Skill power of the pushing attack.
     * @param scaling        Scaling stat of the pushing attack.
     * @param attackerFinal  Final stats of the attacker (after getFinalStats()).
     * @return               Overflow damage (0 if no boundary collision).
     */
    [[nodiscard]] int applyPush(Unit *unit,
                                int delta,
                                float skillPower,
                                ScalingStat scaling,
                                const Stats &attackerFinal);

    [[nodiscard]] Unit *getAt(int position) const;
    [[nodiscard]] int getPosition(const Unit *unit) const; ///< -1 if not found.
    [[nodiscard]] int getCount() const;

  private:
    std::array<Unit *, kMaxSlots> m_slots{};
    int m_count{0};

    void shiftForward(int fromPosition);
    void shiftBackward(int fromPosition);
};