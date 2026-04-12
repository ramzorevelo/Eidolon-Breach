#include "Actions/SkillAction.h"
#include "Entities/PlayableCharacter.h"
#include "Entities/Party.h"
#include "Core/CombatConstants.h"
#include <algorithm>
#include <string>
#include "Core/CombatUtils.h"
#include "Core/ActionUtils.h"

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
    Party& /*allies*/,
    Party& enemies,
    std::optional<TargetInfo> target)
{
    user.useSp(1);
    user.gainEnergy(30);
    return ActionUtils::executeDamageAction(user, enemies, target, m_damage,
        CombatConstants::kSkillToughDmg);
}