/**
 * @file Inventory.cpp
 * @brief Inventory implementation.
 */

#include "Items/Inventory.h"
#include <algorithm>

void Inventory::addItem(const Item &item, int quantity)
{
    if (item.type == ItemType::Equipment)
    {
        m_equipment.push_back(item);
        return;
    }

    // Consumable: find existing stack or create one.
    auto it = std::find_if(m_consumables.begin(), m_consumables.end(),
                           [&item](const std::pair<Item, int> &entry)
                           { return entry.first.id == item.id; });

    if (it != m_consumables.end())
        it->second += quantity;
    else
        m_consumables.emplace_back(item, quantity);
}

bool Inventory::removeItem(std::string_view itemId, int quantity)
{
    auto it = std::find_if(m_consumables.begin(), m_consumables.end(),
                           [itemId](const std::pair<Item, int> &entry)
                           { return entry.first.id == itemId; });

    if (it == m_consumables.end() || it->second < quantity)
        return false;

    it->second -= quantity;
    if (it->second == 0)
        m_consumables.erase(it);
    return true;
}

int Inventory::getQuantity(std::string_view itemId) const
{
    auto it = std::find_if(m_consumables.begin(), m_consumables.end(),
                           [itemId](const std::pair<Item, int> &entry)
                           { return entry.first.id == itemId; });
    return (it != m_consumables.end()) ? it->second : 0;
}

const std::vector<std::pair<Item, int>> &Inventory::getConsumables() const
{
    return m_consumables;
}

const std::vector<Item> &Inventory::getEquipment() const
{
    return m_equipment;
}