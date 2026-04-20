/**
 * @file UltimateAction.cpp
 * @brief UltimateAction implementation.
 */

#include "Actions/UltimateAction.h"
#include "Core/CombatConstants.h"
#include "Core/CombatUtils.h"
#include "Entities/Party.h"
#include "Entities/PlayableCharacter.h"

UltimateAction::UltimateAction()
    : m_data{ActionData{
          .skillPower = 4.0f,
          .scaling = ScalingStat::ATK,
          .spCost = 0,
          .momentumCost = 100,
          .momentumGain = 0,
          .toughnessDamage = CombatConstants::kUltToughDmg,
          .targetMode = TargetMode::SingleEnemy,
          .affinity = Affinity::Blaze}}
{
}

std::string UltimateAction::label() const
{
    return "Ultimate (full Momentum -> 0 | +5 Momentum)";
}

bool UltimateAction::isAvailable(const PlayableCharacter &user,
                                 const Party & /*party*/) const
{
    return user.isUltimateReady();
}

ActionResult UltimateAction::execute(PlayableCharacter &user,
                                     Party & /*allies*/,
                                     Party &enemies,
                                     std::optional<TargetInfo> target)
{
    user.resetEnergy();
    user.gainEnergy(kEnergyRefund);

    ActionResult result{ActionResult::Type::Damage, 0};
    if (target && target->type == TargetInfo::Type::Enemy)
    {
        Unit *t{enemies.getUnitAt(target->index)};
        if (t && t->isAlive())
        {
            result.value = CombatUtils::calculateDamage(m_data.skillPower,
                                                        user.getFinalStats(),
                                                        t->getFinalStats(),
                                                        m_data.scaling);
            t->takeDamage(result.value);
            t->applyToughnessHit(m_data.toughnessDamage);
        }
    }
    return result;
}

Affinity UltimateAction::getAffinity() const
{
    return m_data.affinity;
}
const ActionData &UltimateAction::getActionData() const
{
    return m_data;
}