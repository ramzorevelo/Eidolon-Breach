#pragma once
/**
 * @file ActionData.h
 * @brief Data-driven parameters for every player action (see §2.3.2).
 */

#include "Core/Affinity.h"
#include "Core/ScalingStat.h"
#include "Core/TargetMode.h"

/**
 * @brief Describes an action's resource costs, gains, and targeting behaviour.
 *
 * Stored on each IAction. Actions read from this struct in execute() to apply
 * momentum/SP changes and select targets — avoiding hardcoded values in
 * concrete action classes.
 */
struct ActionData
{
    float skillPower{1.0f};                ///< Damage coefficient (1.0 = 100% of scaling stat).
    ScalingStat scaling{ScalingStat::ATK}; 
    int spCost{0};                         
    int momentumCost{0};                   ///< Deducted on use.
    int momentumGain{0};                   ///< Added after use.
    int toughnessDamage{0};                
    TargetMode targetMode{TargetMode::SingleEnemy};
    Affinity affinity{Affinity::Aether};
};