#pragma once
/**
 * @file Vex.h
 * @brief Stance ID constants and BehaviorSignal → stance mapping for Vex.
 *        Vex is a Terra Conduit. Her three Stances reflect shield/support,
 *        SP-generation, and high-Exposure sacrifice patterns.
 */

#include "Core/BehaviorSignal.h"
#include <string_view>

namespace VexStances
{
constexpr std::string_view kBastion = "vex_bastion";     ///< Shield/Methodical stance.
constexpr std::string_view kResonance = "vex_resonance"; ///< SP surplus / Supportive stance.
constexpr std::string_view kFracture = "vex_fracture";   ///< High-Exposure / Sacrificial stance.
} // namespace VexStances

namespace VexIds
{
constexpr std::string_view kId = "vex";
} // namespace VexIds