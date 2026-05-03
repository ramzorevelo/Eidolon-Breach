#pragma once
/**
 * @file DungeonDefinition.h
 * @brief Difficulty preset and data descriptor for a named dungeon tier.
 */
#include <string>
#include <vector>   

enum class DungeonDifficulty
{
    Normal,
    Hard,
    Nightmare
};

struct DungeonDefinition
{
    std::string id{};
    std::string name{};
    std::string description{};
    int enemyLevel{1};
    int recommendedPlayerLevel{1};
    int unlockPlayerLevel{1};
    int numFloors{5};
    DungeonDifficulty difficulty{DungeonDifficulty::Normal};
    /**
     * @brief Ordered list of node type strings for a fixed Classic dungeon.
     *        Valid strings: "battle", "elite", "boss", "rest", "treasure", "event".
     *        When empty the dungeon uses the procedural generator (EidolonLabyrinth).
     */
    std::vector<std::string> fixedLayout{};
};