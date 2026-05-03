#pragma once
/**
 * @file DungeonTable.h
 * @brief Ordered list of Classic mode dungeon tiers.
 *        Dungeons unlock when: playerLevel >= unlockPlayerLevel
 *        AND the previous dungeon ID is in MetaProgress::clearedDungeonIds
 *        (the first dungeon is always accessible regardless).
 */
#include "Dungeon/DungeonDefinition.h"
#include <vector>

namespace DungeonTable
{
[[nodiscard]] inline const std::vector<DungeonDefinition> &getClassicDungeons()
{
    static const std::vector<DungeonDefinition> kDungeons{
        {"dungeon_01", "The Breach Gate",
         "A lone enemy guards the entrance. Learn the basics of combat.",
         1, 1, 1, 2, DungeonDifficulty::Normal},
        {"dungeon_02", "Fissure Approach",
         "Enemies here resist simple attacks. Exploit their weaknesses.",
         2, 1, 1, 3, DungeonDifficulty::Normal},
        {"dungeon_03", "Resonance Hollow",
         "Your actions build a shared field. Coordinate affinities.",
         3, 2, 1, 4, DungeonDifficulty::Normal},
        {"dungeon_04", "Exposure Rift",
         "The breach warps your senses. Manage your risk carefully.",
         5, 3, 3, 5, DungeonDifficulty::Normal},
        {"dungeon_05", "Deep Rift",
         "Elite threats demand full coordination.",
         8, 5, 5, 7, DungeonDifficulty::Normal},
        {"dungeon_06", "Fracture Zone",
         "A full encounter — every system is in play.",
         12, 8, 8, 9, DungeonDifficulty::Hard},
        {"dungeon_07", "The Inner Breach",
         "The guardian awaits at the core.",
         18, 12, 12, 11, DungeonDifficulty::Hard},
    };
    return kDungeons;
}
} // namespace DungeonTable 