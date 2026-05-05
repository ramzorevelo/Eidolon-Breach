/**
 * @file EmberCallAction.cpp
 * @brief EmberCallAction implementation.
 */

#include "Characters/Lyra/EmberCallAction.h"
#include "Entities/Party.h"
#include "Entities/PlayableCharacter.h"

EmberCallAction::EmberCallAction()
    : m_data{ActionData{
          .skillPower = 0.0f,
          .scaling = ScalingStat::ATK,
          .spCost = kSpCost,
          .energyCost = 0,
          .energyGain = 0,
          .toughnessDamage = 0,
          .targetMode = TargetMode::Self,
          .affinity = Affinity::Blaze,
          .category = ActionCategory::Slot}}
{
}

std::string EmberCallAction::label() const
{
    return "Ember Call (25 SP - Summon Ignis)";
}

bool EmberCallAction::isAvailable(const PlayableCharacter &user,
                                  const Party &party) const
{
    return user.canAffordSp(kSpCost, party);
}

ActionResult EmberCallAction::execute(PlayableCharacter &user,
                                      Party &allies,
                                      Party & /*enemies*/,
                                      std::optional<TargetInfo> /*target*/)
{
    allies.useSp(kSpCost);

    ActionResult result{ActionResult::Type::Skip, 0};
    result.spCost = kSpCost;
    result.actionAffinity = Affinity::Blaze;
    result.flavorText = user.getName() + " calls forth Ignis!";
    result.summonEffect = SummonEffect{std::string{Ignis::kId}, 3,
                                       user.getFinalStats().atk};
    return result;
}

Affinity EmberCallAction::getAffinity() const
{
    return Affinity::Blaze;
}

const ActionData &EmberCallAction::getActionData() const
{
    return m_data;
}