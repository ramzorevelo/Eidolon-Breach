#pragma once
/**
 * @file Inventory.h
 * @brief Shared party inventory: stackable consumables, non-stackable equipment, gold.
 */

#include "Items/Item.h"
#include <string_view>
#include <utility>
#include <vector>

/**
 * @brief Holds consumables (with stack counts) and equipment (single instances).
 *        Gold is a plain int member — not an Item.
 *
 * Consumables are keyed by Item::id. Stacking adds to the count.
 * Equipment items are stored individually; no stacking.
 */
class Inventory
{
  public:
    /**
     * @brief Add item(s) to the inventory.
     *
     * Consumables: if an entry with the same id exists, the count is incremented
     * by quantity; otherwise a new entry is created.
     * Equipment: always appended as a new entry (no stacking).
     *
     * @param item     The item definition to add.
     * @param quantity Number of copies (ignored for Equipment — always 1).
     */
    void addItem(const Item &item, int quantity = 1);

    /**
     * @brief Remove quantity copies of a consumable by id.
     * @return true if the item existed and was decremented successfully;
     *         false if not found or insufficient quantity.
     *         Equipment removal is not supported via this method.
     */
    bool removeItem(std::string_view itemId, int quantity = 1);
    /**
     * @brief Remove the equipment entry at the given 0-based index.
     *        No-op if index is out of range.
     */
    void removeEquipmentAt(std::size_t index);
    /**
     * @brief Returns the stack count for consumable itemId, or 0 if not present.
     *        Always returns 0 for equipment ids.
     */
    [[nodiscard]] int getQuantity(std::string_view itemId) const;

    /** @return Read-only view of all consumable stacks (Item, count) pairs. */
    [[nodiscard]] const std::vector<std::pair<Item, int>> &getConsumables() const;

    /** @return Read-only view of all equipped items (non-stackable). */
    [[nodiscard]] const std::vector<Item> &getEquipment() const;

    int gold{0};

  private:
    std::vector<std::pair<Item, int>> m_consumables{}; ///< (item definition, stack count)
    std::vector<Item> m_equipment{};                   ///< Non-stackable equipment
};
