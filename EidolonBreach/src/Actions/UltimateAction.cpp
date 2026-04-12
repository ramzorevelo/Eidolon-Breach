#include "Actions/UltimateAction.h"
#include "Entities/PlayableCharacter.h"
#include "Entities/Party.h"
#include "Core/CombatConstants.h"
#include <algorithm>

/**
 * @file UltimateAction.cpp
 * @brief Implementation of the ultimate action.
 */
std::string UltimateAction::label() const
{
    return "Ultimate (uses all Energy | +2 SP)";
}

bool UltimateAction::isAvailable(const PlayableCharacter& user) const
{
    return user.ultimateReady();
}

ActionResult UltimateAction::execute(PlayableCharacter& user,
    Party&                    /*allies*/,
    Party& enemies,
    std::optional<TargetInfo> target)
{
    user.resetEnergy();
    user.gainSp(2);

    ActionResult result;
    result.type = ActionResult::Type::Damage;
    result.value = 0;

    if (target && target->type == TargetInfo::Type::Enemy)
    {
        Unit* t = enemies.getUnitAt(target->index);
        if (t && t->isAlive())
        {
            constexpr int basePower = 60;
            float def = static_cast<float>(t->getStats().def);
            float K = CombatConstants::kDefScalingK;
            result.value = std::max(1, static_cast<int>(
                basePower * (1.0f - def / (def + K))));

            t->takeDamage(result.value);
            t->applyToughnessHit(CombatConstants::kUltToughDmg);
        }
    }
    return result;
}