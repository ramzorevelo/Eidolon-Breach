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
inline constexpr int kArchSkillUnlockLevel{20};
constexpr int kSlot1UnlockLevel{20};
inline constexpr int kSlot2UnlockLevel{40}; 

// Consumable cooldown
constexpr int kConsumableCooldownTurns{1};
constexpr float kBrokenDamageBonus{1.5f};
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
constexpr int kMaxEchoes{5};         
constexpr int kInsightPerSignalPoints{3}; 
constexpr int kBondTrialInsightBonus{10}; 

constexpr int kMaxPlayerCharacters{3};

// Character XP — non-linear formula: xpToLevel(n) = floor(kXpLevelBase * n^kXpLevelExponent)
inline constexpr float kXpLevelBase{50.0f};
inline constexpr float kXpLevelExponent{1.5f};

// Player (account) XP
inline constexpr int kPlayerXpDungeonBase{30};       
inline constexpr int kPlayerXpFirstClearBonus{100};  
inline constexpr float kPlayerXpLevelBase{50.0f};
inline constexpr float kPlayerXpLevelExponent{1.5f};
inline constexpr float kCharBattleXpMultiplier{5.0f};
inline constexpr float kCharBattleXpLevelScale{0.2f};
inline constexpr float kPlayerXpLevelScale{0.1f};

// Resonating proc effects — fires for any character at Exposure >= 50
inline constexpr int kResonatingBurnDamage{5};
inline constexpr int kResonatingBurnDuration{2};
inline constexpr float kResonatingSlowPct{0.20f};
inline constexpr int kResonatingSlowDuration{2};
inline constexpr int kResonatingTempestEnergy{15};
inline constexpr float kResonatingTerraShieldPct{0.10f};
inline constexpr int kResonatingTerraShieldDuration{2};

// Surging proc effects — fires for any archetype at Exposure >= 75
inline constexpr int kSurgingStrikerDivisor{4};
inline constexpr int kSurgingConduitSp{20};
inline constexpr int kSurgingDebuffExtendTurns{1};
inline constexpr float kSurgingAnchorShieldPct{0.15f};
inline constexpr int kSurgingAnchorShieldDuration{2};

// Resonance Field trigger effects — fires on any RF trigger by affinity
inline constexpr int kRFBlazeBurnDamage{10};
inline constexpr int kRFBlazeBurnDuration{2};
inline constexpr float kRFFrostSlowPct{0.30f};
inline constexpr int kRFFrostSlowDuration{2};
inline constexpr int kRFTempestEnergy{20};
inline constexpr int kRFTerraShieldAmount{30};
inline constexpr int kRFTerraShieldDuration{2};
inline constexpr int kRFMoltenLatticeShield{15};
inline constexpr int kRFMoltenLatticeDuration{1};
inline constexpr int kRFArcticSurgeSlowDuration{3};

// RF contribution scale by action category
inline constexpr float kRFScaleBasic{1.0f};
inline constexpr float kRFScaleArch{1.5f};
inline constexpr float kRFScaleSlot{1.25f};
inline constexpr float kRFScaleUltimate{2.0f};

// Exposure: Labyrinth character triggers
inline constexpr int kLabyrinthExposureOnKill{4};
inline constexpr int kLabyrinthExposureOnSlot{3};
inline constexpr int kLabyrinthExposureOnDebuff{3};

// Exposure: hit-based source
inline constexpr int kExposureOnHit{3};

// Breachborn burst effects — fires for any character that enters Breachborn
inline constexpr int kBreachbornBlazeBurnDamage{10};
inline constexpr int kBreachbornBlazeBurnDuration{3};
inline constexpr float kBreachbornFrostSlowPct{0.50f};
inline constexpr int kBreachbornFrostSlowDuration{3};
inline constexpr int kBreachbornTempestEnergy{20};
inline constexpr int kBreachbornTerraShieldAmount{25};
inline constexpr int kBreachbornTerraShieldDuration{3};
inline constexpr int kBreachbornAetherContribution{30};

// Breachborn window duration — applies to all characters
inline constexpr int kBreachbornDurationTurns{3};

// Contextual signal thresholds — used by Battle::applyContextualSignals
inline constexpr int kLowHpPctNumerator{30};
inline constexpr int kLowHpPctDenominator{100};
inline constexpr int kLowExposureThreshold{40};
inline constexpr int kConsecutiveLowExposureTurns{3};

// AV-based turn order — all values are placeholders tuned in v0.9.5.
inline constexpr float kBaseAG{10000.0f};        // base action gauge
inline constexpr float kBreakSuppressPct{1.0f};  // approx. one full turn skip
inline constexpr float kTempestHastenPct{0.20f}; // RF Tempest hasten
inline constexpr float kAvModBaseline{1.0f};
inline constexpr float kAvModResonating{0.95f};
inline constexpr float kAvModSurging{0.90f};
inline constexpr float kAvModBreachborn{0.75f};
inline constexpr float kAvModFractured{1.10f};

} // namespace CombatConstants