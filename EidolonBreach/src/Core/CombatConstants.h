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
inline constexpr int kSlot2UnlockLevel{40}; 

// Consumable cooldown
constexpr int kConsumableCooldownTurns{1};
constexpr float kBrokenDamageBonus{1.5f}; ///< HP damage multiplier while enemy is broken.
// Exposure thresholds
constexpr int kExposureThreshold50{50};
constexpr int kExposureThreshold75{75};
constexpr int kExposureThreshold100{100};

// Depth and environmental modifiers
constexpr int kFloorDepthExposureModifier{5};
constexpr int kEliteExposureSpike{15};
constexpr float kFloorAffinityToughnessBonus{0.10f};
constexpr float kFloorAffinityResonanceBonus{0.10f};
constexpr int kPurgeExposureReduction{30};

// Stance crystallization
constexpr int kCrystallizationThreshold{10};

// Supportive signal surplus threshold
constexpr int kSupportiveSpSurplusThreshold{20};
constexpr int kMaxFormationSlots{5};
constexpr int kMaxActiveSummons{2};

// Character leveling (Classic mode)
constexpr int kMaxEchoes{5};         ///< Maximum Resonance Echoes per character.

constexpr int kMaxPlayerCharacters{3}; ///< Maximum Synchrons in a player party.

// Character XP — non-linear formula: xpToLevel(n) = floor(kXpLevelBase * n^kXpLevelExponent)
inline constexpr float kXpLevelBase{50.0f};
inline constexpr float kXpLevelExponent{1.5f};

// Player (account) XP
inline constexpr int kPlayerXpDungeonBase{30};       ///< Base player XP per dungeon completion.
inline constexpr int kPlayerXpFirstClearBonus{100};  ///< One-time bonus on first clear of a dungeon.
inline constexpr float kPlayerXpLevelBase{100.0f};   ///< kXpLevelBase equivalent for player level.
inline constexpr float kPlayerXpLevelExponent{1.8f}; ///< Steeper curve than character XP.
} // namespace CombatConstants