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
        // ── Dungeon 1: Basic combat ───────────────────────────────────────────
        // One battle. No other mechanics in play yet.
        // Player learns: HP bars, basic attack, turn order, winning a fight.
        {
            "dungeon_01",
            "The Breach Gate",
            "A lone enemy guards the entrance. Learn the basics of combat.",
            /*enemyLevel*/ 1,
            /*recommendedPlayer*/ 1,
            /*unlockPlayerLevel*/ 1,
            /*numFloors*/ 1,
            DungeonDifficulty::Normal,
            /*fixedLayout*/ {"battle"},
        },
        // ── Dungeon 2: Toughness break ────────────────────────────────────────
        // Two battles. Enemies have notable toughness bars.
        // Player learns: toughness gauge, break, broken damage bonus.
        {
            "dungeon_02",
            "Fissure Approach",
            "Enemies resist simple attacks. Strike their weak points to break them.",
            /*enemyLevel*/ 2,
            /*recommendedPlayer*/ 1,
            /*unlockPlayerLevel*/ 1,
            /*numFloors*/ 2,
            DungeonDifficulty::Normal,
            /*fixedLayout*/ {"battle", "battle"},
        },
        // ── Dungeon 3: Rest node + SP / Slot Skills ───────────────────────────
        // Battle → Rest → Battle. First Rest node in the game.
        // Player learns: SP generation (basic attack), Slot Skill SP cost, rest options.
        {
            "dungeon_03",
            "Resonance Hollow",
            "The breach opens wider. Rest sites offer relief between encounters.",
            /*enemyLevel*/ 3,
            /*recommendedPlayer*/ 2,
            /*unlockPlayerLevel*/ 2,
            /*numFloors*/ 3,
            DungeonDifficulty::Normal,
            /*fixedLayout*/ {"battle", "rest", "battle"},
        },
        // ── Dungeon 4: Resonance Field ────────────────────────────────────────
        // Five floors. By floor 4 the field has plausibly triggered at least once.
        // Player learns: affinity voting, gauge fill, trigger effects, floor affinity.
        {
            "dungeon_04",
            "Resonance Depths",
            "Your actions resonate together. Coordinate affinities for greater power.",
            /*enemyLevel*/ 4,
            /*recommendedPlayer*/ 3,
            /*unlockPlayerLevel*/ 3,
            /*numFloors*/ 5,
            DungeonDifficulty::Normal,
            /*fixedLayout*/ {"battle", "battle", "rest", "battle", "battle"},
        },
        // ── Dungeon 5: Exposure + Vent + Vestiges ─────────────────────────────
        // Six floors. Depth modifier adds Exposure each battle.
        // First Treasure node awards a vestige.
        // Player learns: Exposure gauge, Vent action, Purge at Rest, vestiges.
        {
            "dungeon_05",
            "Exposure Rift",
            "The breach warps your senses. Manage risk carefully — or vent the pressure.",
            /*enemyLevel*/ 6,
            /*recommendedPlayer*/ 4,
            /*unlockPlayerLevel*/ 5,
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
        // ── Dungeon 6: Elite encounters + deeper vestiges ─────────────────────
        // Seven floors. First Elite node. Post-elite vestige reward.
        // Player learns: elite node spike, Corrupted vestiges, vestige discard.
        {
            "dungeon_06",
            "Deep Rift",
            "Elite threats demand full coordination. Defeat them for powerful rewards.",
            /*enemyLevel*/ 9,
            /*recommendedPlayer*/ 6,
            /*unlockPlayerLevel*/ 7,
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
        // ── Dungeon 7: First boss ─────────────────────────────────────────────
        // Nine floors. Guaranteed Rest before boss.
        // Player has seen every core mechanic before this fight.
        {
            "dungeon_07",
            "The Inner Breach",
            "Every system in play. The guardian of the breach awaits at the core.",
            /*enemyLevel*/ 14,
            /*recommendedPlayer*/ 9,
            /*unlockPlayerLevel*/ 10,
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