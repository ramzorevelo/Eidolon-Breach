#pragma once
/**
 * @file ResourceStats.h
 * @brief Per-character Energy resource (not subject to getFinalStats() modifiers).
 *
 * Energy gates the Ultimate (threshold = 100, resets to 0 on use).
 * Starts at 0 each battle. Does not carry over between battles.
 */

struct ResourceStats
{
    int energy{};
    int maxEnergy{100};
};
