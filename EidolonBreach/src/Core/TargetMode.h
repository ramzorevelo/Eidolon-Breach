#pragma once
/**
 * @file TargetMode.h
 * @brief How an action selects its targets.
 */

/**
 * @brief Declares which units an action may target.
 *
 * Used by ActionData and by PlayableCharacter::selectTarget() to determine
 * the valid pool for ← / → target cycling .
 */
enum class TargetMode
{
    // Player selects one
    SingleEnemy, 
    SingleAlly,  

    // Auto-select entire pool
    AllEnemies, ///< All alive enemies
    AllAllies,  ///< All alive allies 
    Self,       
    SingleAny,  // Enemy or ally. 

    // Formation-based: player picks center, hits center + up to 2 adjacent.
    SplashEnemy, 
    SplashAlly,  

    // Random: no player selection, Battle picks from alive pool.
    RandomEnemy,    // HitCount ignored.
    RandomAlly,     // HitCount ignored.
    RandomXEnemies, // Reads ActionData::hitCount.
    RandomXAllies,  // Reads ActionData::hitCount.

    // Bounce: player picks first target; bounces to hitCount random targets.
    BounceEnemy, 
    BounceAlly,  
};