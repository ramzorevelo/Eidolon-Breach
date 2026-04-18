#include "Actions/SkillAction.h"
#include "Core/ActionUtils.h"
#include "Core/CombatConstants.h"
#include "Core/CombatUtils.h"
#include "Entities/Party.h"
#include "Entities/PlayableCharacter.h"

namespace
{
constexpr int kSpCost{25};
constexpr int kEnergyGain{15};
} // namespace

SkillAction::SkillAction(int damage) : m_damage{damage} {}

std::string SkillAction::label() const
{
    return "Skill (-25 SP | +15 Energy)";
}

bool SkillAction::isAvailable(const PlayableCharacter & /*user*/, const Party &party) const
{
    return party.getSp() >= kSpCost;
}

ActionResult SkillAction::execute(PlayableCharacter &user,
                                  Party &allies,
                                  Party &enemies,
                                  std::optional<TargetInfo> target)
{
    allies.useSp(kSpCost);
    user.gainMomentum(kEnergyGain);
    return ActionUtils::executeDamageAction(user, enemies, target,
                                            m_damage, CombatConstants::kSkillToughDmg);
}