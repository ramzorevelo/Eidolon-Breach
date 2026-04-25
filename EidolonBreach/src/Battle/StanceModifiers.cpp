/**
 * @file StanceModifiers.cpp
 * @brief StanceModifiers implementation. Add new character entries to each
 *        function as new Synchrons are added in Phase 9.
 */

#include "Battle/StanceModifiers.h"
#include "Battle/BattleState.h"
#include "Characters/Lyra.h"
#include "Core/Affinity.h"
#include "Entities/PlayableCharacter.h"
#include <algorithm>

namespace StanceModifiers
{

std::string_view resolveStanceId(std::string_view characterId,
                                 BehaviorSignal dominantSignal)
{
    if (characterId == LyraIds::kStriker)
    {
        if (dominantSignal == BehaviorSignal::Aggressive)
            return LyraStances::kPredator;
        if (dominantSignal == BehaviorSignal::Methodical)
            return LyraStances::kConflagration;
        if (dominantSignal == BehaviorSignal::Sacrificial)
            return LyraStances::kEmber;
        // Supportive and Reactive resolve to Predator as a fallback for Lyra.
        return LyraStances::kPredator;
    }
    return {}; // Unrecognised character — no stance.
}

int applyResonanceModifier(std::string_view stanceId,
                           const PlayableCharacter & /*pc*/,
                           Affinity actionAffinity,
                           int baseAmount,
                           BattleState & /*state*/)
{
    if (stanceId.empty())
        return baseAmount;

    // --- Lyra: Predator ---
    // Basic attacks add +1 to the Blaze Resonance vote via an extra contribution point.
    // This manifests as +1 gauge contribution when actionAffinity == Blaze.
    if (stanceId == LyraStances::kPredator)
    {
        constexpr int kBlazeBonus{1};
        if (actionAffinity == Affinity::Blaze)
            return baseAmount + kBlazeBonus;
        return baseAmount;
    }

    // --- Lyra: Conflagration ---
    // Each consecutive Blaze action adds +8 gauge flat.
    // Turn-number tracking is a Phase 9 refinement; Phase 8 stub: always +8 for Blaze.
    if (stanceId == LyraStances::kConflagration)
    {
        constexpr int kChainBonus{8};
        if (actionAffinity == Affinity::Blaze)
            return baseAmount + kChainBonus;
        return baseAmount;
    }

    // --- Lyra: Ember ---
    // At high Exposure (>= 60), +20% contribution.
    // Exposure check requires a PC reference — handled in applyResonanceContribution
    // in Battle.cpp which passes pc. For now, stub: always +2 for any affinity.
    if (stanceId == LyraStances::kEmber)
    {
        constexpr int kEmberBonus{2};
        return std::max(0, baseAmount + kEmberBonus);
    }

    return baseAmount;
}

} // namespace StanceModifiers