#pragma once
/**
 * @file Item.h
 * @brief Value type for all items: consumables, equipment, currency, and key items.
 *        ItemEffect is a typed variant — add new effect types here as needed.
 */

#include "Core/Affinity.h"
#include <optional>
#include <string>
#include <variant>
#include <vector>


enum class ItemType
{
    Consumable,
    Equipment,
    Currency,
    Key
};
enum class EquipSlot
{
    Weapon,
    Armor,
    Accessory
};
enum class ItemRarity
{
    Common,
    Rare,
    Epic
};

/**
 * @brief Flat additive stat modifier applied as a pre-pass before IStatusEffect mods.
 *        stat identifies which field of Stats to modify; amount may be negative.
 */
struct StatModifier
{
    enum class StatType
    {
        ATK,
        DEF,
        HP,
        SPD
    };
    StatType stat{StatType::ATK};
    int amount{0};
};

/**
 * @brief Adds amount to PlayableCharacter::m_resonanceContribution at equip time.
 *        Applied once at equip, reversed at unequip. Not recalculated per-turn.
 */
struct ResonanceModifier
{
    int amount{0};
};

/**
 * @brief Reduces HP damage taken by a fixed amount when the incoming action's
 *        affinity matches. Applied in Unit::takeDamage() before the DEF formula.
 *        (Flat reduction, not percentage. Percentage comes from IStatusEffect.)
 */
struct AffinityResistance
{
    Affinity affinity{Affinity::Blaze};
    int flatReduction{0};
};

/**
 * @brief Heals the target for amount HP when a consumable is used.
 *        Applied by UseConsumableAction::execute().
 */
struct ConsumableHeal
{
    int amount{0};
};

/**
 * @brief Modifies the target character's Exposure by delta.
 *        Negative = reduce Exposure (e.g. Purification Vial).
 *        Applied by UseConsumableAction::execute().
 */
struct ModifyExposure
{
    int delta{0};
};

/** Typed union of all possible item effects. */
using ItemEffect = std::variant<StatModifier,
                                ResonanceModifier,
                                AffinityResistance,
                                ConsumableHeal,
                                ModifyExposure>;


/**
 * @brief Value type representing one item definition.
 *        Consumables stack (stackSize > 1 allowed).
 *        Equipment does not stack (stackSize always 1).
 */
struct Item
{
    std::string id{};
    std::string name{};
    ItemType type{ItemType::Consumable};
    std::optional<EquipSlot> equipSlot{}; ///< Set for Equipment only.
    ItemRarity rarity{ItemRarity::Common};
    std::vector<ItemEffect> effects{};
    int goldValue{0};
    int stackSize{1}; ///< Always 1 for Equipment.
};