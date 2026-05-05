/**
 * @file StanceModifiers.cpp
 * @brief StanceModifiers implementation. Add new character entries to each
 *        function as new Synchrons are added in Phase 9.
 */

#include "Battle/StanceModifiers.h"
#include "Battle/BattleState.h"
#include "Characters/Lyra/Lyra.h"
#include "Characters/Vex/Vex.h"
#include "Characters/Zara/Zara.h"
#include "Core/Affinity.h"
#include "Entities/PlayableCharacter.h"
#include <algorithm>

namespace StanceModifiers
{

std::string_view resolveStanceId(std::string_view characterId,
                                 BehaviorSignal dominantSignal)
{
    if (characterId == LyraIds::kId)
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
    if (characterId == VexIds::kId)
    {
        if (dominantSignal == BehaviorSignal::Methodical)
            return VexStances::kBastion;
        if (dominantSignal == BehaviorSignal::Supportive)
            return VexStances::kResonance;
        if (dominantSignal == BehaviorSignal::Sacrificial)
            return VexStances::kFracture;
        return VexStances::kBastion; // Aggressive / Reactive fallback
    }
    if (characterId == ZaraIds::kId)
{
    if (dominantSignal == BehaviorSignal::Methodical)
        return ZaraStances::kGlacial;
    if (dominantSignal == BehaviorSignal::Aggressive)
        return ZaraStances::kShatter;
    if (dominantSignal == BehaviorSignal::Supportive)
        return ZaraStances::kConvergence;
    return ZaraStances::kGlacial; // Sacrificial / Reactive fallback
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

    // --- Vex: Bastion ---
    // Terra actions contribute +2 extra — steady defensive energy.
    if (stanceId == VexStances::kBastion)
    {
        constexpr int kTerraBonus{2};
        if (actionAffinity == Affinity::Terra)
            return baseAmount + kTerraBonus;
        return baseAmount;
    }

    // --- Vex: Resonance ---
    // SP-surplus play generates +3 extra on Terra actions.
    if (stanceId == VexStances::kResonance)
    {
        constexpr int kTerraBonus{3};
        if (actionAffinity == Affinity::Terra)
            return baseAmount + kTerraBonus;
        return baseAmount;
    }

    // --- Vex: Fracture ---
    // High-Exposure commitment releases raw energy — any affinity gains +4.
    if (stanceId == VexStances::kFracture)
    {
        constexpr int kFractureBonus{4};
        return std::max(0, baseAmount + kFractureBonus);
    }

    // --- Zara: Glacial ---
    // Frost control chains deepen the field — +3 to Frost contributions.
    if (stanceId == ZaraStances::kGlacial)
    {
        constexpr int kFrostBonus{3};
        if (actionAffinity == Affinity::Frost)
            return baseAmount + kFrostBonus;
        return baseAmount;
    }

    // --- Zara: Shatter ---
    // Break-hunting sharpens the edge — +2 to Frost, oriented toward toughness lanes.
    if (stanceId == ZaraStances::kShatter)
    {
        constexpr int kFrostBonus{2};
        if (actionAffinity == Affinity::Frost)
            return baseAmount + kFrostBonus;
        return baseAmount;
    }

    // --- Zara: Convergence ---
    // SP surplus channels into the field — +2 to all affinities.
    if (stanceId == ZaraStances::kConvergence)
    {
        constexpr int kConvergeBonus{2};
        return std::max(0, baseAmount + kConvergeBonus);
    }

    return baseAmount;
}

} // namespace StanceModifiers