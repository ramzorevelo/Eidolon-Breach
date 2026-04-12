#include "Actions/UltimateAction.h"
#include "Entities/PlayableCharacter.h"
#include "Entities/Party.h"
#include "Core/CombatConstants.h"
#include <algorithm>
#include "Core/CombatUtils.h"
#include "Core/ActionUtils.h"

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
    Party& /*allies*/,
    Party& enemies,
    std::optional<TargetInfo> target)
{
    user.resetEnergy();
    user.gainSp(2);
    constexpr int ultimateBasePower{ 60 };
    return ActionUtils::executeDamageAction(user, enemies, target, ultimateBasePower,
        CombatConstants::kUltToughDmg);
}