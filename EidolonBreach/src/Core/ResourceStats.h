#pragma once

/**
 * @file ResourceStats.h
 * @brief Per‑character resources not subject to getFinalStats() modifiers.
 */

/** Energy resource for PlayableCharacter. Not affected by buffs/debuffs. */
struct ResourceStats
{
    int energy{};
    int maxEnergy{};
};