#pragma once
/**
 * @file CombatUtils.h
 * @brief Damage calculation utilities.
 */

#include "Core/CombatConstants.h"
#include "Core/ScalingStat.h"
#include "Core/Stats.h"
#include <algorithm>

namespace CombatUtils
{
    /**
     * @brief Flat-damage overload: used by enemy attacks that return a raw int.
     * @param flatDamage  Raw damage value from performAttack().
     * @param defenderDef Defender's effective DEF stat.
     * @return Final damage (minimum 1).
     */
    [[nodiscard]] inline int calculateDamage(int flatDamage, int defenderDef)
    {
        float def{static_cast<float>(defenderDef)};
        float K{CombatConstants::kDefScalingK};
        return std::max(1, static_cast<int>(flatDamage * (1.0f - def / (def + K))));
    }

    /**
     * @brief ActionData overload: used by player actions with skill power and scaling.
     *
     * damage = max(1, skillPower * scalingStat * (1 - DEF / (DEF + K)))
     *
     * @param skillPower      Coefficient from ActionData (e.g. 1.5 = 150% scaling).
     * @param attackerFinal   Attacker's stats after getFinalStats() (buffs applied).
     * @param defenderFinal   Defender's stats after getFinalStats() (debuffs applied).
     * @param scaling         Which attacker stat to use as the base.
     * @return Final damage (minimum 1).
     */
    [[nodiscard]] inline int calculateDamage(float skillPower,
                                             const Stats &attackerFinal,
                                             const Stats &defenderFinal,
                                             ScalingStat scaling = ScalingStat::ATK)
    {
        float scalingStat{};
        switch (scaling)
        {
        case ScalingStat::ATK:
            scalingStat = static_cast<float>(attackerFinal.atk);
            break;
        case ScalingStat::DEF:
            scalingStat = static_cast<float>(attackerFinal.def);
            break;
        case ScalingStat::HP:
            scalingStat = static_cast<float>(attackerFinal.hp);
            break;
        }
        float def{static_cast<float>(defenderFinal.def)};
        float K{CombatConstants::kDefScalingK};
        float raw{skillPower * scalingStat};
        float mit{1.0f - def / (def + K)};
        return std::max(1, static_cast<int>(raw * mit));
    }
}