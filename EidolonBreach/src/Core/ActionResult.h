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

    int spGained{0};      ///< SP added to party pool by this action.
    int exposureDelta{0}; ///< Exposure change for the acting character.
};