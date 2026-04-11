#include "Actions/UltimateAction.h"
#include "Core/CombatConstants.h"

std::string UltimateAction::label() const
{
    return "Ultimate (60 dmg | 30 break | uses all energy)";
}

bool UltimateAction::isAvailable(const Player& player) const
{
    return player.ultimateReady();
}

ActionResult UltimateAction::execute(Player& player, Enemy& enemy)
{
    ActionResult result = player.useUltimate();
    enemy.takeDamage(result.value);
    enemy.reduceToughness(CombatConstants::kUltToughDmg);
    return result;
}