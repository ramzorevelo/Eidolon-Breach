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
constexpr int kXpPerLevel{50};       ///< Flat XP cost per level. Level N needs (N-1)*50 total XP.
constexpr int kXpStandardBattle{10}; ///< XP awarded per standard battle victory.
constexpr int kXpBossBattle{50};     ///< XP awarded for boss victory.
constexpr int kMaxEchoes{5};         ///< Maximum Resonance Echoes per character.
} // namespace CombatConstants