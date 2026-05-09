#pragma once
/**
 * @file Lyra.h
 * @brief Stance ID constants and BehaviorSignal → stance mapping for Lyra.
 *        Lyra is a Blaze Striker. Her three Stances crystallize from different
 *        behavioral patterns tracked by RunContext::RunCharacterState::signalCounts.
 */

#include "Core/BehaviorSignal.h"
#include <string_view>

namespace LyraStances
{
constexpr std::string_view kPredator = "lyra_predator";
constexpr std::string_view kConflagration = "lyra_conflagration";
constexpr std::string_view kEmber = "lyra_ember";
} // namespace LyraStances

namespace LyraIds
{
constexpr std::string_view kId = "lyra"; 
} // namespace LyraIds

namespace LyraConstants
{
/// Fracture state: self-DoT as a fraction of max HP each turn.
constexpr float kFractureSelfDotPct{0.05f};

/// Breachborn per-action bonus: true damage = result.value / kBreachbornActionBonusDivisor.
constexpr int kBreachbornActionBonusDivisor{2};
constexpr int kBreachbornActionBurnDamage{5};
constexpr int kBreachbornActionBurnDuration{1};
} // namespace LyraConstants