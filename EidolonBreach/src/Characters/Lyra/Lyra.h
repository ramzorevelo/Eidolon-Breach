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