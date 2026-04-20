#pragma once
/**
 * @file CombatConstants.h
 * @brief Global constants for combat calculations.
 */

namespace CombatConstants
{
constexpr int kBasicToughDmg{10};
constexpr int kSkillToughDmg{25};
constexpr int kUltToughDmg{30};
constexpr float kDefScalingK{100.0f};

// Arch Skill cooldown (Hotfix 4)
constexpr int kArchSkillCooldownTurns{2};

// Slot unlock levels
constexpr int kSlot1UnlockLevel{20};
constexpr int kSlot2UnlockLevel{40};

// Consumable cooldown (Hotfix 5)
constexpr int kConsumableCooldownTurns{1};
} // namespace CombatConstants