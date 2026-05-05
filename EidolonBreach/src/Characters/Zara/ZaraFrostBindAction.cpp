/**
 * @file ZaraFrostbindAction.cpp
 * @brief ZaraFrostbindAction implementation.
 */
#include "Characters/Zara/ZaraFrostbindAction.h"
#include "Core/Effects/SlowEffect.h"
#include "Entities/Party.h"
#include "Entities/PlayableCharacter.h"

ZaraFrostbindAction::ZaraFrostbindAction()
    : m_data{ActionData{
          .skillPower = 0.0f,
          .scaling = ScalingStat::ATK,
          .spCost = kSpCost,
          .energyCost = 0,
          .energyGain = 0,
          .toughnessDamage = kToughnessDamage,
          .targetMode = TargetMode::SingleEnemy,
          .affinity = Affinity::Frost,
          .category = ActionCategory::Slot}}
{
}

std::string ZaraFrostbindAction::label() const
{
    return "Frostbind (25 SP - Slow one enemy)";
}

bool ZaraFrostbindAction::isAvailable(const PlayableCharacter &user,
                                      const Party &party) const
{
    return user.canAffordSp(kSpCost, party);
}

ActionResult ZaraFrostbindAction::execute(PlayableCharacter &user,
                                          Party &allies,
                                          Party &enemies,
                                          std::optional<TargetInfo> target)
{
    allies.useSp(kSpCost);

    ActionResult result{ActionResult::Type::Skip, 0};
    result.spCost = kSpCost;
    result.actionAffinity = Affinity::Frost;

    if (target && target->type == TargetInfo::Type::Enemy)
    {
        Unit *t{enemies.getUnitAt(target->index)};
        if (t && t->isAlive())
        {
            t->applyEffect(std::make_unique<SlowEffect>(kSlowRatio, kSlowDuration));
            t->applyToughnessHit(kToughnessDamage, Affinity::Frost);
            result.toughnessDamage = kToughnessDamage;
            result.targetEnemyIndex = static_cast<int>(target->index);
            result.flavorText = user.getName() + " binds the enemy in frost!";
        }
    }
    return result;
}

Affinity ZaraFrostbindAction::getAffinity() const
{
    return Affinity::Frost;
}
const ActionData &ZaraFrostbindAction::getActionData() const
{
    return m_data;
}