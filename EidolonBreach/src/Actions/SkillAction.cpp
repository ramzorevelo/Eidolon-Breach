#include "Actions/SkillAction.h"
#include "Entities/PlayableCharacter.h"
#include "Entities/Party.h"
#include "Core/CombatConstants.h"
#include <algorithm>
#include <string>

/**
 * @file SkillAction.cpp
 * @brief Implementation of the skill action.
 */
SkillAction::SkillAction(int damage) : m_damage{ damage } {}

std::string SkillAction::label() const
{
    return "Skill (-1 SP | +30 Energy)";
}

bool SkillAction::isAvailable(const PlayableCharacter& user) const
{
    return user.getSp() >= 1;
}

ActionResult SkillAction::execute(PlayableCharacter& user,
    Party&                    /*allies*/,
    Party& enemies,
    std::optional<TargetInfo> target)
{
    user.useSp(1);
    user.gainEnergy(30);

    ActionResult result;
    result.type = ActionResult::Type::Damage;
    result.value = 0;

    if (target && target->type == TargetInfo::Type::Enemy)
    {
        Unit* t = enemies.getUnitAt(target->index);
        if (t && t->isAlive())
        {
            float def = static_cast<float>(t->getStats().def);
            float K = CombatConstants::kDefScalingK;
            result.value = std::max(1, static_cast<int>(
                m_damage * (1.0f - def / (def + K))));

            t->takeDamage(result.value);
            t->applyToughnessHit(CombatConstants::kSkillToughDmg);
        }
    }
    return result;
}