#pragma once
/**
 * @file DungeonTable.h
 * @brief Ordered list of Classic mode dungeon tiers.
 *        Dungeons unlock sequentially: clear dungeon N to unlock dungeon N+1.
 *        The first dungeon is always accessible. Player account level no longer
 *        gates Classic dungeon access.
 */
#include "Dungeon/DungeonDefinition.h"
#include <vector>

namespace DungeonTable
{
[[nodiscard]] inline const std::vector<DungeonDefinition> &
getClassicDungeons()
{
    static const std::vector<DungeonDefinition> kDungeons{

        // Chapter 1 — The Breach Gate
        {
            "dungeon_01", "The Breach Gate", "A lone spirit guards the entrance. Learn to fight.", 1, 1, 1, DungeonDifficulty::Normal, {"battle"}, 1, {{"spirit_01"}}},
        {"dungeon_02", "Fissure Approach", "Enemies resist simple attacks. Strike weak points to break them.", 2, 1, 2, DungeonDifficulty::Normal, {"battle", "battle"}, 1, {{"spirit_01", "breach_slime"}, {"breach_slime"}}},
        {"dungeon_03", "Resonance Hollow", "The breach opens wider. Rest when you can.", 3, 2, 4, DungeonDifficulty::Normal, {"battle", "battle", "rest", "boss"}, 1, {{"breach_slime"}, {"breach_slime", "spirit_01"}, {}, {"gate_warden"}}},

        // Chapter 2 — The Resonance Depths
        {
            "dungeon_04", "Resonance Depths", "Your actions resonate together. Coordinate affinities.", 4, 3, 5, DungeonDifficulty::Normal, {"battle", "battle", "battle", "battle", "battle"}, 2, {{"spirit_01", "breach_slime"}, {"frost_wisp"}, {"iron_sentinel"}, {"breach_slime", "frost_wisp"}, {"frost_wisp", "iron_sentinel"}}},
        {"dungeon_05", "Exposure Rift", "The breach warps your senses. Manage risk or vent the pressure.", 6, 4, 4, DungeonDifficulty::Normal, {"battle", "elite", "rest", "battle"}, 2, {{"breach_slime", "frost_wisp"}, {"stone_golem"}, {}, {"iron_sentinel", "breach_slime"}}},
        {"dungeon_06", "Surge Point", "The resonance peaks. Ride the wave or be consumed.", 9, 6, 4, DungeonDifficulty::Normal, {"battle", "battle", "rest", "boss"}, 2, {{"iron_sentinel", "frost_wisp"}, {"stone_golem", "breach_slime"}, {}, {"surge_herald"}}},

        // Chapter 3 — The Inner Sanctum
        {
            "dungeon_07", "Sanctum Approach", "The sanctum demands precision. No room for error.", 11, 8, 3, DungeonDifficulty::Normal, {"battle", "elite", "battle"}, 3, {{"iron_sentinel", "iron_sentinel"}, {"depth_golem"}, {"frost_wisp", "iron_sentinel", "breach_slime"}}},
        {"dungeon_08", "Sanctum Depths", "Two elites, one rest. Both hit hard.", 13, 10, 4, DungeonDifficulty::Normal, {"battle", "elite", "rest", "elite"}, 3, {{"iron_sentinel", "iron_sentinel"}, {"depth_golem", "breach_slime"}, {}, {"breach_warden"}}},
        {"dungeon_09", "Sanctum Throne", "The guardian waits. You arrive already taxed.", 15, 12, 3, DungeonDifficulty::Normal, {"elite", "rest", "boss"}, 3, {{"depth_golem", "frost_wisp"}, {}, {"sanctum_sentinel"}}},

        // Chapter 4 — The Core
        {
            "dungeon_10", "The Core", "The final breach. No rest. No mercy.", 18, 15, 3, DungeonDifficulty::Normal, {"battle", "elite", "boss"}, 4, {{"iron_sentinel", "iron_sentinel", "breach_slime"}, {"breach_warden", "shard_wraith"}, {"void_herald"}}},
    };
    return kDungeons;
}
} // namespace DungeonTable 