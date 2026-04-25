#pragma once
/**
 * @file Summon.h
 * @brief A player-side Manifestation. Acts automatically on its own turn.
 *        Contributes to the Resonance Field at 50% of the summoner's contribution.
 *        Constructed from a SummonDefinition via SummonRegistry.
 */

#include "Entities/Unit.h"
#include "Summons/SummonDefinition.h"
#include <cstddef>
#include <optional>

class Summon : public Unit
{
  public:
    /**
     * @param def                  The summon's data. Not owned — SummonRegistry owns it.
     * @param summonerContribution The summoner's resonanceContribution at spawn time.
     *                             Stored as floor(contribution / 2) per GDD §3.3.
     */
    Summon(const SummonDefinition &def, int summonerContribution);

    ActionResult takeTurn(Party &allies, Party &enemies, BattleState &state) override;

    /** @return true — identifies this unit as a Summon for Battle's cap check. */
    [[nodiscard]] bool isSummon() const override
    {
        return true;
    }

    /** @return 50% of summoner's resonanceContribution (floor). */
    [[nodiscard]] int getResonanceContribution() const override;

    /** @return true when the fixed-duration summon has expired. */
    [[nodiscard]] bool isExpired() const;

    /**
     * @brief Decrement remaining duration by 1, clamped to 0.
     *        No-op when duration is nullopt (summon lives until destroyed).
     *        Auto-removal on expiry is deferred to Phase 9.
     */
    void tickDuration();

  private:
    const SummonDefinition *m_definition{nullptr};
    int m_resonanceContribution{};
    std::optional<int> m_remainingTurns{};
    std::size_t m_actionIndex{0}; ///< Round-robin index into the action pool.
};