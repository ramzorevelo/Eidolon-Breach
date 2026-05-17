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
    int numFloors{5};
    DungeonDifficulty difficulty{DungeonDifficulty::Normal};
    /**
     * @brief Ordered list of node type strings for a fixed Classic dungeon.
     *        Valid strings: "battle", "elite", "boss", "rest", "treasure", "event".
     *        When empty the dungeon uses the procedural generator (EidolonLabyrinth).
     */
    std::vector<std::string> fixedLayout{};
    
    /**
     * @brief Chapter number (1–4). Classic mode only.
     *        Used to determine which dungeons unlock Hard/Nightmare together.
     */
    int chapter{0};

    /**
     * @brief Enemy ID list per floor for Classic authored content.
     *        Entry i lists the enemy IDs spawned on floor i.
     *        When non-empty, buildFixedGraph instantiates these directly
     *        via EnemyRegistry; EncounterTable is bypassed.
     *        Rest node floors use an empty inner vector.
     */
    std::vector<std::vector<std::string>> fixedEnemyGroups{};
};