/**
 * @file ZaraUltimateAction.cpp
 * @brief ZaraUltimateAction implementation.
 */

#include "Actions/ZaraUltimateAction.h"
#include "Core/CombatUtils.h"
#include "Core/Effects/SlowEffect.h"
#include "Entities/Party.h"
#include "Entities/PlayableCharacter.h"

ZaraUltimateAction::ZaraUltimateAction()
    : m_data{ActionData{
          .skillPower = 2.0f,
          .scaling = ScalingStat::ATK,
          .spCost = 0,
          .momentumCost = 100,
          .momentumGain = 0,
          .toughnessDamage = kToughnessDamage,
          .targetMode = TargetMode::AllEnemies,
          .affinity = Affinity::Frost,
          .category = ActionCategory::Ultimate}}
{
}

std::string ZaraUltimateAction::label() const
{
    return "Arctic Shatter (full Energy -> 0 | +5 Energy)";
}

bool ZaraUltimateAction::isAvailable(const PlayableCharacter &user,
                                     const Party & /*party*/) const
{
    return user.isUltimateReady();
}

ActionResult ZaraUltimateAction::execute(PlayableCharacter &user,
                                         Party & /*allies*/,
                                         Party &enemies,
                                         std::optional<TargetInfo> /*target*/)
{
    user.resetEnergy();
    user.gainEnergy(kEnergyRefund);

    int totalDamage{0};
    for (Unit *u : enemies.getAliveUnits())
    {
        const int dmg{CombatUtils::calculateDamage(
            m_data.skillPower, user.getFinalStats(),
            u->getFinalStats(), m_data.scaling)};
        u->takeDamage(dmg);
        u->applyEffect(std::make_unique<SlowEffect>(kSlowRatio, kSlowDuration));
        u->applyToughnessHit(kToughnessDamage, Affinity::Frost);
        totalDamage += dmg;
    }

    ActionResult result{ActionResult::Type::Damage, totalDamage};
    result.actionAffinity = Affinity::Frost;
    result.flavorText = user.getName() + " shatters the field with Arctic Shatter!";
    return result;
}

Affinity ZaraUltimateAction::getAffinity() const
{
    return Affinity::Frost;
}

const ActionData &ZaraUltimateAction::getActionData() const
{
    return m_data;
}