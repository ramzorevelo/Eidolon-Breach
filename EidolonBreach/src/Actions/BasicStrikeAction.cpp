#include "Actions/BasicStrikeAction.h"
#include "Core/CombatConstants.h"

std::string BasicStrikeAction::label() const
{
    return "Basic Strike (15 dmg | 10 break | +1 SP | +20 energy)";
}

ActionResult BasicStrikeAction::execute(Player& player, Enemy& enemy)
{
    ActionResult result = player.basicAttack();
    enemy.takeDamage(result.value);
    enemy.reduceToughness(CombatConstants::kBasicToughDmg);
    return result;
}