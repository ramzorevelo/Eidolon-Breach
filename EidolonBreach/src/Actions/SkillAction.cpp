#include "Actions/SkillAction.h"
#include "Core/CombatConstants.h"

std::string SkillAction::label() const
{
    return "Skill (28 dmg | 25 break | -1 SP | +30 energy)";
}

bool SkillAction::isAvailable(const Player& player) const
{
    return player.getSp() >= 1;
}

ActionResult SkillAction::execute(Player& player, Enemy& enemy)
{
    ActionResult result = player.useSkill();
    enemy.takeDamage(result.value);
    enemy.reduceToughness(CombatConstants::kSkillToughDmg);
    return result;
}