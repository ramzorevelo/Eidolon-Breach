/**
 * @file UseConsumableAction.cpp
 * @brief UseConsumableAction implementation.
 */

#include "Actions/UseConsumableAction.h"
#include "Entities/Party.h"
#include "Entities/PlayableCharacter.h"
#include <algorithm>
#include <variant>

UseConsumableAction::UseConsumableAction(Item item)
    : m_item{std::move(item)},
      m_data{ActionData{
          .skillPower = 0.0f,
          .scaling = ScalingStat::ATK,
          .spCost = 0,
          .momentumCost = 0,
          .momentumGain = 0,
          .toughnessDamage = 0,
          .targetMode = TargetMode::Self,
          .affinity = Affinity::Aether,
          .category = ActionCategory::Consumable}}
{
}

std::string UseConsumableAction::label() const
{
    return "Use: " + m_item.name;
}

bool UseConsumableAction::isAvailable(const PlayableCharacter &user,
                                      const Party &party) const
{
    return user.canUseConsumable() &&
           party.getInventory().getQuantity(m_item.id) > 0;
}

ActionResult UseConsumableAction::execute(PlayableCharacter &user,
                                          Party &allies,
                                          Party &enemies,
                                          std::optional<TargetInfo> target)
{
    (void)enemies;

    // Determine whether any effect is multi-turn (regen consumable, etc.).
    // Phase 4: no multi-turn consumables defined yet — always false.
    constexpr bool kIsMultiTurn{false};
    user.consumeConsumableAction(kIsMultiTurn);

    // Decrement the inventory.
    allies.getInventory().removeItem(m_item.id, 1);

    ActionResult result{ActionResult::Type::Skip, 0};

    for (const ItemEffect &effect : m_item.effects)
    {
        if (const auto *heal = std::get_if<ConsumableHeal>(&effect))
        {
            // Apply to the target ally, or to self if no target specified.
            Unit *targetUnit{nullptr};
            if (target && target->type == TargetInfo::Type::Ally)
                targetUnit = allies.getUnitAt(target->index);
            if (!targetUnit)
                targetUnit = &user;

            if (targetUnit->isAlive())
            {
                targetUnit->heal(heal->amount);
                result.type = ActionResult::Type::Heal;
                result.value = heal->amount;
            }
        }
        else if (const auto *exp = std::get_if<ModifyExposure>(&effect))
        {
            // Exposure always applies to self.
            user.modifyExposure(exp->delta);
            result.exposureDelta += exp->delta;
        }
    }

    result.flavorText = "Used " + m_item.name + ".";
    return result;
}

Affinity UseConsumableAction::getAffinity() const
{
    return Affinity::Aether;
}

const ActionData &UseConsumableAction::getActionData() const
{
    return m_data;
}