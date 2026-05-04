/**
 * @file LyraUltimateAction.cpp
 * @brief LyraUltimateAction implementation.
 */

#include "Actions/LyraUltimateAction.h"
#include "Core/CombatConstants.h"
#include "Core/CombatUtils.h"
#include "Core/Effects/BurnEffect.h"
#include "Entities/Party.h"
#include "Entities/PlayableCharacter.h"

LyraUltimateAction::LyraUltimateAction()
    : m_data{ActionData{
          .skillPower = 4.0f,
          .scaling = ScalingStat::ATK,
          .spCost = 0,
          .momentumCost = 100,
          .momentumGain = 0,
          .toughnessDamage = CombatConstants::kUltToughDmg,
          .targetMode = TargetMode::SingleEnemy,
          .affinity = Affinity::Blaze,
          .category = ActionCategory::Ultimate}}
{
}

std::string LyraUltimateAction::label() const
{
    return "Pyre's Ruin (full Energy -> 0 | +5 Energy)";
}

bool LyraUltimateAction::isAvailable(const PlayableCharacter &user,
                                     const Party & /*party*/) const
{
    return user.isUltimateReady();
}

ActionResult LyraUltimateAction::execute(PlayableCharacter &user,
                                         Party & /*allies*/,
                                         Party &enemies,
                                         std::optional<TargetInfo> target)
{
    user.resetEnergy();
    user.gainEnergy(kEnergyRefund);

    ActionResult result{ActionResult::Type::Damage, 0};
    result.actionAffinity = Affinity::Blaze;

    if (target && target->type == TargetInfo::Type::Enemy)
    {
        Unit *t{enemies.getUnitAt(target->index)};
        if (t && t->isAlive())
        {
            result.value = CombatUtils::calculateDamage(
                m_data.skillPower, user.getFinalStats(),
                t->getFinalStats(), m_data.scaling);
            t->takeDamage(result.value);
            t->applyEffect(std::make_unique<BurnEffect>(kBurnDamage, kBurnDuration));
            t->applyToughnessHit(m_data.toughnessDamage, Affinity::Blaze);
            result.toughnessDamage = m_data.toughnessDamage;
            result.targetEnemyIndex = static_cast<int>(target->index);
            result.flavorText = user.getName() + " unleashes Pyre's Ruin!";
        }
    }
    return result;
}

Affinity LyraUltimateAction::getAffinity() const
{
    return Affinity::Blaze;
}

const ActionData &LyraUltimateAction::getActionData() const
{
    return m_data;
}