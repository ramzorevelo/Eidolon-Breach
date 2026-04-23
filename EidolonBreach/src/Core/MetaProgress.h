#pragma once
/**
 * @file MetaProgress.h
 * @brief Cross-run persistent state. Persistence and full fields added in a later commit.
 *        Stub provides enough interface for Dungeon and MapNode to compile.
 */

#include <string_view>

class MetaProgress
{
  public:
    /**
     * @brief Award XP to a character. No-op until XP system is implemented.
     * @param characterId The Unit::getId() of the character receiving XP.
     * @param amount      Amount of XP to award.
     */
    void gainXP(std::string_view characterId, int amount);

    int currency{0};
    int highestFloorReached{0};
};