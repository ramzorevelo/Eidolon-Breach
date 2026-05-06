#pragma once
/**
 * @file ActionData.h
 * @brief Data-driven parameters for every player action.
 */

#include "Core/Affinity.h"
#include "Core/ScalingStat.h"
#include "Core/TargetMode.h"

/**
 * @brief Categorises an action for stance signal tracking and equip-screen validation.
 *        Used by Battle::
 () and UseConsumableAction.
 */
enum class ActionCategory
{
    Basic,      ///< Basic Attack [Q].
    ArchSkill,  ///< Arch Skill [E] — fixed, non-equippable.
    Slot,       ///< Slot Skill [1] or [2] — equippable.
    Ultimate,   ///< Ultimate [R] — fixed, non-equippable.
    Consumable, ///< UseConsumableAction [item menu].
    Vent,       ///< Vent [V].
};

/**
 * @brief Describes an action's resource costs, gains, and targeting behaviour.
 */
struct ActionData
{
    float skillPower{1.0f};
    ScalingStat scaling{ScalingStat::ATK};
    int spCost{0};
    int energyCost{0}; 
    int energyGain{0}; 
    int toughnessDamage{0};
    TargetMode targetMode{TargetMode::SingleEnemy};
    Affinity affinity{Affinity::Aether};
    ActionCategory category{ActionCategory::Slot};
};