#pragma once
/**
 * @file ActionResult.h
 * @brief Result of an action used for UI and game state updates.
 *
 * spGained:      SP added to the party pool by this action (e.g., Basic Attack +15).
 *                Applied by Battle::processActionResult() — not by the action itself.
 * exposureDelta: Change to the acting character's Exposure gauge.
 *                Positive = gain, negative = reduction. Applied by processActionResult().
 *                Corrupted Actions set a positive value; some supportive skills set negative.
 */

#include <string>
#include "Core/Affinity.h"
#include <optional>

/**
 * @brief Signals that the executing action spawned a Manifestation.
 *        Set by summoner skill actions; processed by Battle::processSummonEffect().
 */
struct SummonEffect
{
    std::string summonId{};
    int preferredPosition{3};
    int summonerAtk{0}; ///< Attacker ATK at spawn time, used by scaling actions
};

struct ActionResult
{
    enum class Type
    {
        Damage,
        Heal,
        Charge,
        Skip
    };
    Type type{Type::Damage};
    int value{0};
    std::string flavorText{};
    int spGained{0};
    int exposureDelta{0};
    Affinity actionAffinity{Affinity::Aether};
    /**
     * @brief Base toughness damage dealt by this action (before affinity scaling).
     *        Set by actions that call applyToughnessHit. Read by ToughnessBreakerVestige.
     *        0 if the action deals no toughness damage.
     */
    int toughnessDamage{0};

    /**
     * @brief SP cost paid by this action. Set by Slot Skill actions.
     *        Read by EchoingStrikeVestige to refund after a Resonance Field trigger.
     *        0 for free actions (Basic, Arch Skill, Ultimate).
     */
    int spCost{0};

    /**
     * @brief Index into the enemy party of the primary target, or -1 if no
     *        enemy target was selected (e.g. heals, Vent, Skip).
     *        Used by ToughnessBreakerVestige to locate the target unit.
     */
    int targetEnemyIndex{-1};
    /**
     * @brief Populated by summoner skill actions when a Manifestation is spawned.
     *        Processed by Battle after execute() returns; never set by the action itself.
     */
    std::optional<SummonEffect> summonEffect{};
};