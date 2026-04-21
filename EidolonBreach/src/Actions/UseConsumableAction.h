#pragma once
/**
 * @file UseConsumableAction.h
 * @brief Action that consumes one use of an item from the party inventory.
 *        No SP cost. No Energy gain. Ends the character's turn.
 *        Availability: canUseConsumable() && item still has quantity > 0.
 *
 * Targeting (GDD §4.12, §4.13):
 *   ConsumableHeal  → lowest-HP ally (auto-select, overridable).
 *   ModifyExposure  → self only.
 *   Default         → self.
 */

#include "Actions/IAction.h"
#include "Items/Item.h"
#include <string>

/**
 * @brief Wraps one Item for use as a combat action.
 *        Constructed fresh each time the player selects an item from the menu.
 *        Does NOT own the item — it reads from the Party inventory.
 */
class UseConsumableAction : public IAction
{
  public:
    /**
     * @param item The item to consume. A copy is stored; the original
     *             in the Inventory is decremented by execute().
     */
    explicit UseConsumableAction(Item item);

    std::string label() const override;
    bool isAvailable(const PlayableCharacter &user,
                     const Party &party) const override;
    ActionResult execute(PlayableCharacter &user,
                         Party &allies,
                         Party &enemies,
                         std::optional<TargetInfo> target) override;
    Affinity getAffinity() const override;
    const ActionData &getActionData() const override;

  private:
    Item m_item;
    ActionData m_data;
};