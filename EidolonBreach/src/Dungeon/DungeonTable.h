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
[[nodiscard]] inline const std::vector<DungeonDefinition> &getClassicDungeons()
{
    static const std::vector<DungeonDefinition> kDungeons{
        // One battle. No other mechanics in play yet.
        // Player learns: HP bars, basic attack, turn order, winning a fight.
        {
            "dungeon_01",
            "The Breach Gate",
            "A lone enemy guards the entrance. Learn the basics of combat.",
            /*enemyLevel*/ 1,
            /*recommendedPlayer*/ 1,
            /*numFloors*/ 1,
            DungeonDifficulty::Normal,
            /*fixedLayout*/ {"battle"},
        },
        // Two battles. Enemies have notable toughness bars.
        // Player learns: toughness gauge, break, broken damage bonus.
        {
            "dungeon_02",
            "Fissure Approach",
            "Enemies resist simple attacks. Strike their weak points to break them.",
            /*enemyLevel*/ 2,
            /*recommendedPlayer*/ 1,
            /*numFloors*/ 2,
            DungeonDifficulty::Normal,
            /*fixedLayout*/ {"battle", "battle"},
        },
        // Battle → Rest → Battle. First Rest node in the game.
        // Player learns: SP generation (basic attack), Slot Skill SP cost, rest options.
        {
            "dungeon_03",
            "Resonance Hollow",
            "The breach opens wider. Rest sites offer relief between encounters.",
            /*enemyLevel*/ 3,
            /*recommendedPlayer*/ 2,
            /*numFloors*/ 3,
            DungeonDifficulty::Normal,
            /*fixedLayout*/ {"battle", "rest", "battle"},
        },
        // Five floors. By floor 4 the field has plausibly triggered at least once.
        // Player learns: affinity voting, gauge fill, trigger effects, floor affinity.
        {
            "dungeon_04",
            "Resonance Depths",
            "Your actions resonate together. Coordinate affinities for greater power.",
            /*enemyLevel*/ 4,
            /*recommendedPlayer*/ 3,
            /*numFloors*/ 5,
            DungeonDifficulty::Normal,
            /*fixedLayout*/ {"battle", "battle", "rest", "battle", "battle"},
        },
        // Six floors. Depth modifier adds Exposure each battle.
        // First Treasure node awards a vestige.
        // Player learns: Exposure gauge, Vent action, Purge at Rest, vestiges.
        {
            "dungeon_05",
            "Exposure Rift",
            "The breach warps your senses. Manage risk carefully — or vent the pressure.",
            /*enemyLevel*/ 6,
            /*recommendedPlayer*/ 4,
            /*numFloors*/ 6,
            DungeonDifficulty::Normal,
            /*fixedLayout*/ {
                "battle",
                "battle",
                "rest",
                "battle",
                "treasure",
                "battle",
            },
        },
        // Seven floors. First Elite node. Post-elite vestige reward.
        // Player learns: elite node spike, Corrupted vestiges, vestige discard.
        {
            "dungeon_06",
            "Deep Rift",
            "Elite threats demand full coordination. Defeat them for powerful rewards.",
            /*enemyLevel*/ 9,
            /*recommendedPlayer*/ 6,
            /*numFloors*/ 7,
            DungeonDifficulty::Normal,
            /*fixedLayout*/ {
                "battle",
                "elite",
                "rest",
                "battle",
                "treasure",
                "shop",
                "battle",
            },
        },
        // Nine floors. Guaranteed Rest before boss.
        // Player has seen every core mechanic before this fight.
        {
            "dungeon_07",
            "The Inner Breach",
            "Every system in play. The guardian of the breach awaits at the core.",
            /*enemyLevel*/ 14,
            /*recommendedPlayer*/ 9,
            /*numFloors*/ 9,
            DungeonDifficulty::Hard,
            /*fixedLayout*/ {
                "battle",
                "elite",
                "rest",
                "battle",
                "treasure",
                "shop",
                "elite",
                "rest",
                "boss",
            },
        },
    };
    return kDungeons;
}
} // namespace DungeonTable 