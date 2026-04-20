/**
 * @file BasicStrikeAction.cpp
 * @brief BasicStrikeAction implementation.
 */

#include "Actions/BasicStrikeAction.h"
#include "Core/CombatConstants.h"
#include "Core/CombatUtils.h"
#include "Entities/Party.h"
#include "Entities/PlayableCharacter.h"

BasicStrikeAction::BasicStrikeAction()
    : m_data{ActionData{
          .skillPower = 1.0f,
          .scaling = ScalingStat::ATK,
          .spCost = 0,
          .momentumCost = 0,
          .momentumGain = 0,
          .toughnessDamage = CombatConstants::kBasicToughDmg,
          .targetMode = TargetMode::SingleEnemy,
          .affinity = Affinity::Blaze}}
{
}

std::string BasicStrikeAction::label() const
{
    return "Basic Strike (+15 SP | +25 Energy)";
}

bool BasicStrikeAction::isAvailable(const PlayableCharacter & /*user*/,
                                    const Party & /*party*/) const
{
    return true;
}

ActionResult BasicStrikeAction::execute(PlayableCharacter &user,
                                        Party & /*allies*/,
                                        Party &enemies,
                                        std::optional<TargetInfo> target)
{
    user.gainEnergy(kEnergyGainToUser);

    ActionResult result{ActionResult::Type::Damage, 0};
    result.spGained = kSpGainToParty; // Battle::processActionResult() applies this.

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

Affinity BasicStrikeAction::getAffinity() const
{
    return m_data.affinity;
}

const ActionData &BasicStrikeAction::getActionData() const
{
    return m_data;
}