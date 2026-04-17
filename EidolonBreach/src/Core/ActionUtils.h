#pragma once
/**
 * @file ActionUtils.h
 * @brief Shared helper functions for damage-dealing actions.
 */

#include "Core/ActionResult.h"
#include "Core/CombatConstants.h"
#include "Core/CombatUtils.h"
#include "Entities/Party.h"
#include <optional>

class PlayableCharacter;

namespace ActionUtils
{
/**
 * @brief Executes a damaging action against a single target.
 * @param user The acting character.
 * @param enemies The enemy party.
 * @param target The target selection info.
 * @param basePower Raw damage value.
 * @param toughDmg Toughness damage value.
 * @return An ActionResult populated with the damage dealt.
 */
inline ActionResult executeDamageAction(PlayableCharacter & /*user*/,
                                        Party &enemies,
                                        std::optional<TargetInfo> target,
                                        int basePower,
                                        int toughDmg)
{
    ActionResult result{ActionResult::Type::Damage, 0};
    if (target && target->type == TargetInfo::Type::Enemy)
    {
        Unit *t = enemies.getUnitAt(target->index);
        if (t && t->isAlive())
        {
            result.value = CombatUtils::calculateDamage(basePower, t->getFinalStats().def);
            t->takeDamage(result.value);
            t->applyToughnessHit(toughDmg);
        }
    }
    return result;
}
} // namespace ActionUtils