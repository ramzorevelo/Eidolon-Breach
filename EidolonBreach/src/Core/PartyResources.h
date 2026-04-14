#pragma once

/**
 * @file PartyResources.h
 * @brief Shared resource pool for the player party (SP).
 */

/** Shared SP (Skill Points) pool for the entire player party. */
struct PartyResources
{
    int sp{};
    int maxSp{};
};