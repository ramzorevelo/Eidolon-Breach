#pragma once
/**
 * @file ResourceStats.h
 * @brief Per-character Momentum resource (not subject to getFinalStats() modifiers).
 *
 * Momentum gates Arch Skill (threshold >= 40, costs 40) and Ultimate
 * (threshold = 100, resets to 0). Starts at 0 each battle.
 */

struct ResourceStats
{
    int momentum{};
    int maxMomentum{};
};