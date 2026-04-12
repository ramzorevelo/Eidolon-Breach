#pragma once

/** 
 * @file CombatConstants.h
 * @brief Global constants for combat calculations.
 */
namespace CombatConstants
{
    constexpr int   kBasicToughDmg{ 10 };
    constexpr int   kSkillToughDmg{ 25 };
    constexpr int   kUltToughDmg{ 30 };
    constexpr float kDefScalingK{ 100.0f };  // damage = max(1, base*(1 - DEF/(DEF+K)))
}