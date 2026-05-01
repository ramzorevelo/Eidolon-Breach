/**
 * @file RunContext.cpp
 * @brief RunContext implementation.
 */

#include "Core/RunContext.h"
#include "Core/FieldDiscovery.h"

RunCharacterState &RunContext::getCharacterState(std::string_view unitId)
{
    // emplace returns the existing entry unchanged if the key is already present.
    auto result{m_states.emplace(std::string{unitId}, RunCharacterState{})};
    return result.first->second;
}

const RunCharacterState *RunContext::findCharacterState(std::string_view unitId) const
{
    auto it{m_states.find(unitId)};
    return (it != m_states.end()) ? &it->second : nullptr;
}

void RunContext::recordFieldVotes(Affinity affinity, float votes)
{
    m_fieldVoteTotals[affinity] += votes;
}

float RunContext::getAffinityVoteTotal(Affinity affinity) const
{
    auto it{m_fieldVoteTotals.find(affinity)};
    return (it != m_fieldVoteTotals.end()) ? it->second : 0.0f;
}

void RunContext::reset()
{
    m_states.clear();
    m_fieldVoteTotals.clear();
    activeDiscoveries.clear();
}

void RunContext::checkAndActivateDiscoveries()
{
    // Compute total votes across all affinities.
    float total{0.0f};
    for (const auto &[affinity, votes] : m_fieldVoteTotals)
        total += votes;

    if (total <= 0.0f)
        return;

    const float blazePct{getAffinityVoteTotal(Affinity::Blaze) / total};
    const float frostPct{getAffinityVoteTotal(Affinity::Frost) / total};
    const float tempestPct{getAffinityVoteTotal(Affinity::Tempest) / total};
    const float terraPct{getAffinityVoteTotal(Affinity::Terra) / total};
    const float aetherPct{getAffinityVoteTotal(Affinity::Aether) / total};

    // Molten Lattice: Blaze >= 60% and Terra >= 30%
    if (blazePct >= 0.60f && terraPct >= 0.30f)
        activeDiscoveries.insert(std::string{FieldDiscoveryIds::kMoltenLattice});

    // Arctic Surge: Frost >= 60% and Tempest >= 30%
    if (frostPct >= 0.60f && tempestPct >= 0.30f)
        activeDiscoveries.insert(std::string{FieldDiscoveryIds::kArcticSurge});

    // Lattice Attunement: Aether >= 30% and any single elemental affinity >= 50%
    const bool aetherMet{aetherPct >= 0.30f};
    const bool anyDominant{blazePct >= 0.50f || frostPct >= 0.50f ||
                           tempestPct >= 0.50f || terraPct >= 0.50f};
    if (aetherMet && anyDominant)
        activeDiscoveries.insert(std::string{FieldDiscoveryIds::kLatticeAttunement});
}