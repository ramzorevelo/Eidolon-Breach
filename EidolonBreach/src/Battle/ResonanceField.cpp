/**
 * @file ResonanceField.cpp
 * @brief ResonanceField implementation.
 */

#include "Battle/ResonanceField.h"
#include <algorithm>
#include <sstream>
#include <string>

void ResonanceField::addContribution(Affinity affinity, int amount)
{
    m_gauge = std::min(kGaugeCap, m_gauge + amount);

    if (affinity == Affinity::Aether)
    {
        // Aether contributes kAetherVoteFraction votes to every affinity.
        for (int a{static_cast<int>(Affinity::Blaze)};
             a <= static_cast<int>(Affinity::Aether); ++a)
        {
            m_votes[static_cast<Affinity>(a)] += kAetherVoteFraction;
        }
    }
    else
    {
        m_votes[affinity] += 1.0f;
    }
}

bool ResonanceField::isReady() const
{
    return m_gauge >= kGaugeCap;
}

Affinity ResonanceField::trigger()
{
    Affinity winner{findDominantAffinity()};

    // Resonance Memory: retain kMemoryRatio of winning votes.
    float retained{m_votes[winner] * kMemoryRatio};

    m_votes.clear();
    m_votes[winner] = retained;
    m_gauge = 0;

    return winner;
}

void ResonanceField::reset()
{
    m_gauge = 0;
    m_votes.clear();
}

float ResonanceField::getVotes(Affinity affinity) const
{
    auto it{m_votes.find(affinity)};
    return (it != m_votes.end()) ? it->second : 0.0f;
}

std::string ResonanceField::getVoteSummary() const
{
    std::ostringstream oss{};
    bool first{true};
    for (const auto &[affinity, votes] : m_votes)
    {
        if (votes <= 0.0f)
            continue;
        if (!first)
            oss << ' ';
        oss << affinityToString(affinity) << '(' << votes << ')';
        first = false;
    }
    return oss.str();
}

Affinity ResonanceField::findDominantAffinity() const
{
    Affinity best{Affinity::Blaze};
    float bestVotes{-1.0f};
    for (const auto &[affinity, votes] : m_votes)
    {
        // Higher votes wins; lower enum value breaks ties (deterministic).
        if (votes > bestVotes)
        {
            bestVotes = votes;
            best = affinity;
        }
    }
    return best;
}