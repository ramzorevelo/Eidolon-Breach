#pragma once
/**
 * @file ResonanceField.h
 * @brief Shared gauge tracking affinity votes for player actions.
 */

#include "Core/Affinity.h"
#include <map>
#include <string>

/**
 * @brief Tracks affinity votes and triggers resonance effects.
 *
 * When a player unit acts, addContribution() is called with the action's
 * affinity and the character's resonanceContribution. When the gauge reaches
 * 100, trigger() fires the dominant affinity and retains 25% of its votes
 * (Resonance Memory). Aether adds 0.5 votes to every affinity simultaneously.
 */
class ResonanceField
{
  public:
    static constexpr int kGaugeCap{100};
    static constexpr float kMemoryRatio{0.25f}; // retained after trigger
    static constexpr float kAetherVoteFraction{0.5f};

    /**
     * @brief Record an action's contribution to the gauge.
     *
     * Increments the gauge by amount. Records the action's affinity as votes.
     * Aether actions add kAetherVoteFraction votes to every affinity.
     *
     * @param affinity Affinity of the action used.
     * @param amount   resonanceContribution of the acting character.
     */
    void addContribution(Affinity affinity, int amount);

    /** @return true when the gauge is at or above kGaugeCap. */
    bool isReady() const;

    /**
     * @brief Fire the trigger: return the winning affinity, reset gauge and votes.
     *
     * Retains kMemoryRatio of the winning affinity's votes (Resonance Memory).
     * All other affinity votes reset to 0. Gauge resets to 0.
     *
     * On a vote tie, the affinity with the lower enum value wins deterministically.
     *
     * @return The affinity that triggered.
     */
    [[nodiscard]] Affinity trigger();

    /** Reset the gauge and all votes to zero (e.g. at battle start). */
    void reset();

    /**
     * @brief Human-readable vote summary for console display.
     * @return e.g. "Blaze(5.0) Frost(2.5)" — only affinities with > 0 votes shown.
     */
    [[nodiscard]] std::string getVoteSummary() const;

    int getGauge() const
    {
        return m_gauge;
    }
    float getVotes(Affinity affinity) const;
    /** @return The affinity with the most votes. Lowest enum value breaks ties. */
    [[nodiscard]] Affinity getLeadingAffinity() const;
  private:
    int m_gauge{0};
    std::map<Affinity, float> m_votes{};

    Affinity findDominantAffinity() const;
};