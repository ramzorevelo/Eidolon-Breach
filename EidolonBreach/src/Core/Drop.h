#pragma once
/**
 * @file Drop.h
 * @brief Drop descriptor returned by Enemy::generateDrops().
 *        Variant type: a drop is either a gold amount or an item.
 */

#include <string>

/**
 * @brief Describes one entry in an enemy's drop pool.
 *
 * Type::Gold:          awards goldAmount gold to the party.
 * Type::Item:          awards the item with itemId from the item registry.
 * Type::GuaranteedItem: same as Item but dropChance is always treated as 1.0.
 */
struct Drop
{
    enum class Type
    {
        Gold,
        Item,
        GuaranteedItem
    };

    Type type{Type::Gold};
    int goldAmount{0};
    std::string itemId{};   ///< Used when type is Item or GuaranteedItem.
    float dropChance{1.0f}; ///< 0.0–1.0; 1.0 = always drops.
};