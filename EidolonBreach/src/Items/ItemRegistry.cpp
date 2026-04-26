/**
 * @file ItemRegistry.cpp
 * @brief ItemRegistry implementation.
 */

#include "Items/ItemRegistry.h"
#include "Core/DataLoader.h"
#include <stdexcept>

void ItemRegistry::loadFromJson(const std::string &jsonPath)
{
    const nlohmann::json j{DataLoader::loadJson(jsonPath)};
    for (auto it = j.begin(); it != j.end(); ++it)
        m_items[it.key()] = parseItem(it.key(), it.value());
}

std::optional<Item> ItemRegistry::create(std::string_view itemId) const
{
    auto it = m_items.find(std::string{itemId});
    if (it == m_items.end())
        return std::nullopt;
    return it->second;
}

bool ItemRegistry::contains(std::string_view itemId) const
{
    return m_items.count(std::string{itemId}) > 0;
}

std::size_t ItemRegistry::size() const
{
    return m_items.size();
}

Item ItemRegistry::parseItem(const std::string &id, const nlohmann::json &j)
{
    Item item{};
    item.id = id;
    item.name = j.at("name").get<std::string>();

    const std::string typeStr{j.at("type").get<std::string>()};
    if (typeStr == "Consumable")
        item.type = ItemType::Consumable;
    else if (typeStr == "Equipment")
        item.type = ItemType::Equipment;
    else if (typeStr == "Currency")
        item.type = ItemType::Currency;
    else
        item.type = ItemType::Key;

    if (j.contains("equipSlot"))
    {
        const std::string slotStr{j.at("equipSlot").get<std::string>()};
        if (slotStr == "Weapon")
            item.equipSlot = EquipSlot::Weapon;
        else if (slotStr == "Armor")
            item.equipSlot = EquipSlot::Armor;
        else if (slotStr == "Accessory")
            item.equipSlot = EquipSlot::Accessory;
    }

    const std::string rarityStr{j.value("rarity", "Common")};
    if (rarityStr == "Rare")
        item.rarity = ItemRarity::Rare;
    else if (rarityStr == "Epic")
        item.rarity = ItemRarity::Epic;
    else
        item.rarity = ItemRarity::Common;

    item.stackSize = j.value("stackSize", 1);
    item.goldValue = j.value("goldValue", 0);

    for (const auto &effectJson : j.value("effects", nlohmann::json::array()))
        item.effects.push_back(parseEffect(effectJson));

    return item;
}

ItemEffect ItemRegistry::parseEffect(const nlohmann::json &j)
{
    const std::string type{j.at("type").get<std::string>()};

    if (type == "ConsumableHeal")
        return ConsumableHeal{j.at("amount").get<int>()};

    if (type == "ModifyExposure")
        return ModifyExposure{j.at("delta").get<int>()};

    if (type == "ResonanceModifier")
        return ResonanceModifier{j.at("amount").get<int>()};

    if (type == "StatModifier")
    {
        StatModifier sm{};
        const std::string stat{j.at("stat").get<std::string>()};
        if (stat == "ATK")
            sm.stat = StatModifier::StatType::ATK;
        else if (stat == "DEF")
            sm.stat = StatModifier::StatType::DEF;
        else if (stat == "HP")
            sm.stat = StatModifier::StatType::HP;
        else if (stat == "SPD")
            sm.stat = StatModifier::StatType::SPD;
        sm.amount = j.at("amount").get<int>();
        return sm;
    }

    if (type == "AffinityResistance")
    {
        AffinityResistance ar{};
        const std::string aff{j.at("affinity").get<std::string>()};
        if (aff == "Blaze")
            ar.affinity = Affinity::Blaze;
        else if (aff == "Frost")
            ar.affinity = Affinity::Frost;
        else if (aff == "Tempest")
            ar.affinity = Affinity::Tempest;
        else if (aff == "Terra")
            ar.affinity = Affinity::Terra;
        else
            ar.affinity = Affinity::Aether;
        ar.flatReduction = j.at("flatReduction").get<int>();
        return ar;
    }

    throw std::runtime_error{"ItemRegistry: unknown effect type '" + type + "'"};
}