#include "Actions/BasicStrikeAction.h"
#include "Entities/PlayableCharacter.h"
#include "Entities/Party.h"
#include "Core/CombatConstants.h"
#include "Core/CombatUtils.h"
#include <algorithm>
#include <cmath>
#include "Core/ActionUtils.h"

/**
 * @file BasicStrikeAction.cpp
 * @brief Standard single‑target physical attack.
 */
std::string BasicStrikeAction::label() const
{
    return "Basic Strike (+1 SP | +20 Energy)";
}

ActionResult BasicStrikeAction::execute(PlayableCharacter& user,
    Party& /*allies*/,
    Party& enemies,
    std::optional<TargetInfo> target)
{
    user.gainSp(1);
    user.gainEnergy(20);
    constexpr int basicStrikeBasePower{ 15 };
    return ActionUtils::executeDamageAction(user, enemies, target, basicStrikeBasePower,
        CombatConstants::kBasicToughDmg);
}