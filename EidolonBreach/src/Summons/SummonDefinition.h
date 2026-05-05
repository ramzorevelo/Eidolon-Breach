#pragma once
/**
 * @file SummonDefinition.h
 * @brief Data definition for one summon type. Owned exclusively by SummonRegistry.
 *        SummonEffect references definitions by string ID at spawn time.
 */

#include "Core/Stats.h"
#include "Summons/SummonAction.h"
#include <optional>
#include <string>
#include <vector>

struct SummonDefinition
{
    std::string id{};
    std::string displayName{};
    Stats baseStats{};             ///< hp, maxHp, atk, def, spd. Set at registration.
    std::optional<int> duration{}; ///< Turns before auto-expiry. nullopt = until destroyed.
    bool canTaunt{false};          
    bool autoMoveFront{false};     
    std::vector<SummonAction> actions{};
};