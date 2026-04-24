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
          .spCost = 0,
          .momentumCost = 0,
          .momentumGain = 0,
          .toughnessDamage = CombatConstants::kSkillToughDmg,
          .targetMode = TargetMode::SingleEnemy,
          .affinity = Affinity::Blaze,
          .category = ActionCategory::ArchSkill}}
{
}

std::string SkillAction::label() const
{
    return "Arch Skill (2-turn cooldown)";
}

bool SkillAction::isAvailable(const PlayableCharacter &user, const Party & /*party*/) const
{
    return user.isArchSkillReady();
}

ActionResult SkillAction::execute(PlayableCharacter &user,
                                  Party &allies,
                                  Party &enemies,
                                  std::optional<TargetInfo> target)
{
    allies.useSp(m_data.spCost);
    user.consumeArchSkill();

    ActionResult result{ActionResult::Type::Damage, 0};
    result.spCost = m_data.spCost; // recorded so EchoingStrikeVestige can refund   
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
            t->applyToughnessHit(m_data.toughnessDamage, m_data.affinity);
            result.toughnessDamage = m_data.toughnessDamage;
            result.targetEnemyIndex = static_cast<int>(target->index);
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