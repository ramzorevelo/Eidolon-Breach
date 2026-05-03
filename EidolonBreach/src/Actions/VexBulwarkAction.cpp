/**
 * @file VexBulwarkAction.cpp
 * @brief VexBulwarkAction implementation.
 */
#include "Actions/VexBulwarkAction.h"
#include "Battle/TargetingHeuristics.h"
#include "Core/Effects/ShieldEffect.h"
#include "Entities/Party.h"
#include "Entities/PlayableCharacter.h"

VexBulwarkAction::VexBulwarkAction()
    : m_data{ActionData{
          .skillPower = 0.0f,
          .scaling = ScalingStat::DEF,
          .spCost = kSpCost,
          .momentumCost = 0,
          .momentumGain = 0,
          .toughnessDamage = 0,
          .targetMode = TargetMode::SingleAlly,
          .affinity = Affinity::Terra,
          .category = ActionCategory::Slot}}
{
}

std::string VexBulwarkAction::label() const
{
    return "Earthen Shield (25 SP - Shield one ally)";
}

bool VexBulwarkAction::isAvailable(const PlayableCharacter &user,
                                   const Party &party) const
{
    return user.canAffordSp(kSpCost, party);
}

ActionResult VexBulwarkAction::execute(PlayableCharacter &user,
                                       Party &allies,
                                       Party & /*enemies*/,
                                       std::optional<TargetInfo> target)
{
    allies.useSp(kSpCost);

    std::size_t targetIdx{
        TargetingHeuristics::defaultAllyTarget(TargetMode::SingleAlly, allies)};
    if (target && target->type == TargetInfo::Type::Ally)
        targetIdx = target->index;

    Unit *t{allies.getUnitAt(targetIdx)};
    if (t && t->isAlive())
        t->applyEffect(std::make_unique<ShieldEffect>(kShieldAmount, kShieldDuration));

    ActionResult result{ActionResult::Type::Skip, 0};
    result.spCost = kSpCost;
    result.actionAffinity = Affinity::Terra;
    result.flavorText = user.getName() + " raises an earthen shield!";
    return result;
}

Affinity VexBulwarkAction::getAffinity() const
{
    return Affinity::Terra;
}
const ActionData &VexBulwarkAction::getActionData() const
{
    return m_data;
}