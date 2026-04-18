#include "Actions/UltimateAction.h"
#include "Core/ActionUtils.h"
#include "Core/CombatConstants.h"
#include "Core/CombatUtils.h"
#include "Entities/Party.h"
#include "Entities/PlayableCharacter.h"

namespace
{
constexpr int kEnergyRefund{5};
constexpr int kBasePower{60};
} // namespace

std::string UltimateAction::label() const
{
    return "Ultimate (full Energy -> 0 | +5 Energy)";
}

bool UltimateAction::isAvailable(const PlayableCharacter &user, const Party & /*party*/) const
{
    return user.isUltimateReady();
}

ActionResult UltimateAction::execute(PlayableCharacter &user,
                                     Party & /*allies*/,
                                     Party &enemies,
                                     std::optional<TargetInfo> target)
{
    user.resetMomentum();
    user.gainMomentum(kEnergyRefund);
    return ActionUtils::executeDamageAction(user, enemies, target,
                                            kBasePower, CombatConstants::kUltToughDmg);
}