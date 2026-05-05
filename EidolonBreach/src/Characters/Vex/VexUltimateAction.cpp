/**
 * @file VexUltimateAction.cpp
 * @brief VexUltimateAction implementation.
 */

#include "Characters/Vex/VexUltimateAction.h"
#include "Core/Effects/ShieldEffect.h"
#include "Entities/Party.h"
#include "Entities/PlayableCharacter.h"

VexUltimateAction::VexUltimateAction()
    : m_data{ActionData{
          .skillPower = 0.0f,
          .scaling = ScalingStat::ATK,
          .spCost = 0,
          .energyCost = 100,
          .energyGain = 0,
          .toughnessDamage = kToughnessDamage,
          .targetMode = TargetMode::AllAllies,
          .affinity = Affinity::Terra,
          .category = ActionCategory::Ultimate}}
{
}

std::string VexUltimateAction::label() const
{
    return "Resonant Bulwark (full Energy -> 0 | +5 Energy)";
}

bool VexUltimateAction::isAvailable(const PlayableCharacter &user,
                                    const Party & /*party*/) const
{
    return user.isUltimateReady();
}

ActionResult VexUltimateAction::execute(PlayableCharacter &user,
                                        Party &allies,
                                        Party &enemies,
                                        std::optional<TargetInfo> /*target*/)
{
    user.resetEnergy();
    user.gainEnergy(kEnergyRefund);

    // Shield all alive allies.
    for (Unit *u : allies.getAliveUnits())
        u->applyEffect(std::make_unique<ShieldEffect>(kShieldAmount, kShieldDuration));

    // Grant SP to the party.
    allies.gainSp(kSpGrant);

    // Apply toughness damage to all alive enemies.
    for (Unit *u : enemies.getAliveUnits())
        u->applyToughnessHit(kToughnessDamage, Affinity::Terra);

    ActionResult result{ActionResult::Type::Skip, 0};
    result.actionAffinity = Affinity::Terra;
    result.flavorText = user.getName() + " raises Resonant Bulwark!";
    return result;
}

Affinity VexUltimateAction::getAffinity() const
{
    return Affinity::Terra;
}

const ActionData &VexUltimateAction::getActionData() const
{
    return m_data;
}