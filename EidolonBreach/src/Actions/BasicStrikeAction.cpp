#include "Actions/BasicStrikeAction.h"
#include "Entities/PlayableCharacter.h"
#include "Entities/Party.h"
#include "Core/CombatConstants.h"
#include <algorithm>
#include <cmath>

/**
 * @file BasicStrikeAction.cpp
 * @brief Standard single‑target physical attack.
 */
std::string BasicStrikeAction::label() const
{
    return "Basic Strike (+1 SP | +20 Energy)";
}

ActionResult BasicStrikeAction::execute(PlayableCharacter& user,
    Party&                    /*allies*/,
    Party& enemies,
    std::optional<TargetInfo> target)
{
    user.gainSp(1);
    user.gainEnergy(20);

    ActionResult result;
    result.type = ActionResult::Type::Damage;
    result.value = 0;

    if (target && target->type == TargetInfo::Type::Enemy)
    {
        Unit* t = enemies.getUnitAt(target->index);
        if (t && t->isAlive())
        {
            constexpr int basePower = 15;
            float def = static_cast<float>(t->getStats().def);
            float K = CombatConstants::kDefScalingK;
            result.value = std::max(1, static_cast<int>(
                basePower * (1.0f - def / (def + K))));

            t->takeDamage(result.value);
            t->applyToughnessHit(CombatConstants::kBasicToughDmg);
        }
    }
    return result;
}