/**
 * @file SkillAction.cpp
 * @brief SkillAction implementation.
 */

#include "Actions/SkillAction.h"
#include "Core/CombatConstants.h"
#include "Core/CombatUtils.h"
#include "Entities/Party.h"
#include "Entities/PlayableCharacter.h"

SkillAction::SkillAction(float skillPower)
    : m_data{ActionData{
          .skillPower = skillPower,
          .scaling = ScalingStat::ATK,
          .spCost = 25,
          .momentumCost = 40,
          .momentumGain = 0,
          .toughnessDamage = CombatConstants::kSkillToughDmg,
          .targetMode = TargetMode::SingleEnemy,
          .affinity = Affinity::Blaze}}
{
}

std::string SkillAction::label() const
{
    return "Arch Skill (-25 SP | -40 Momentum)";
}

bool SkillAction::isAvailable(const PlayableCharacter &user, const Party &party) const
{
    return party.getSp() >= m_data.spCost && user.getEnergy() >= m_data.momentumCost;
}

ActionResult SkillAction::execute(PlayableCharacter &user,
                                  Party &allies,
                                  Party &enemies,
                                  std::optional<TargetInfo> target)
{
    allies.useSp(m_data.spCost);
    user.consumeEnergy(m_data.momentumCost);

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

Affinity SkillAction::getAffinity() const
{
    return m_data.affinity;
}
const ActionData &SkillAction::getActionData() const
{
    return m_data;
}