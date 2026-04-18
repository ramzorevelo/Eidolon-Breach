#pragma once
/**
 * @file ScalingStat.h
 * @brief Which attacker stat is used in the damage formula.
 */

/**
 * @brief Scaling stat for damage calculation (see §2.3.5).
 *
 * Stored in ActionData so ATK-scaling, DEF-scaling, and HP-scaling
 * actions all use the same combat-loop code in CombatUtils.
 */
enum class ScalingStat
{
    ATK, 
    DEF, 
    HP   
};