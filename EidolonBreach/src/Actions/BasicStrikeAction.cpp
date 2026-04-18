#include "Actions/BasicStrikeAction.h"
#include "Core/ActionUtils.h"
#include "Core/CombatConstants.h"
#include "Core/CombatUtils.h"
#include "Entities/Party.h"
#include "Entities/PlayableCharacter.h"

namespace
{
constexpr int kSpGain{15};    // to party pool
constexpr int kEnergyGain{8}; // to self
constexpr int kBasePower{15};
} // namespace

std::string BasicStrikeAction::label() const
{
    return "Basic Strike (+15 SP | +8 Energy)";
}

bool BasicStrikeAction::isAvailable(const PlayableCharacter & /*user*/, const Party & /*party*/) const
{
    return true; // always available
}

ActionResult BasicStrikeAction::execute(PlayableCharacter &user,
                                        Party &allies,
                                        Party &enemies,
                                        std::optional<TargetInfo> target)
{
    allies.gainSp(kSpGain);
    user.gainMomentum(kEnergyGain);
    return ActionUtils::executeDamageAction(user, enemies, target,
                                            kBasePower, CombatConstants::kBasicToughDmg);
}