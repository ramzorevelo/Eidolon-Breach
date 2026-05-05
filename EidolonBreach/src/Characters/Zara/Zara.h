#pragma once
/**
 * @file Zara.h
 * @brief Stance ID constants and BehaviorSignal → stance mapping for Zara.
 *        Zara is a Frost Weaver. Her Stances reflect control-chain, break-focused,
 *        and SP-surplus behavioral patterns.
 */

#include "Core/BehaviorSignal.h"
#include <string_view>

namespace ZaraStances
{
constexpr std::string_view kGlacial = "zara_glacial";         ///< Control chain / Methodical stance.
constexpr std::string_view kShatter = "zara_shatter";         ///< Break-focused / Aggressive stance.
constexpr std::string_view kConvergence = "zara_convergence"; ///< SP-surplus / Supportive stance.
} // namespace ZaraStances

namespace ZaraIds
{
constexpr std::string_view kId = "zara"; 
} // namespace ZaraIds