/**
 * @file RunContext.cpp
 * @brief RunContext implementation.
 */

#include "Core/RunContext.h"

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
}