#pragma once
/**
 * @file RunContext.h
 * @brief Per-run mutable state for Stance crystallization and Field Discovery.
 *        Owned by Dungeon. Injected into BattleState. Reset at run start.
 *        std::less<> enables string_view lookups without constructing std::string keys.
 */

#include "Core/Affinity.h"
#include "Core/BehaviorSignal.h"
#include <map>
#include <optional>
#include <string>

/** Per-character run state used to track Stance crystallization progress. */
struct RunCharacterState
{
    std::map<BehaviorSignal, int> signalCounts{};
    std::optional<std::string> crystallizedStanceId{};
    int synchronicityProgress{0};
    int totalSpGenerated{0};
    int totalSpSpent{0};
    int consecutiveLowExposureTurns{0};
};

class RunContext
{
  public:
    /**
     * @brief Returns state for unitId; creates a default entry if absent.
     * @param unitId The Unit::getId() value of the character.
     */
    RunCharacterState &getCharacterState(std::string_view unitId);

    /**
     * @brief Returns nullptr if unitId has no recorded state.
     */
    [[nodiscard]] const RunCharacterState *findCharacterState(std::string_view unitId) const;

    /** @brief Accumulate affinity votes for Field Discovery tracking. */
    void recordFieldVotes(Affinity affinity, float votes);

    [[nodiscard]] float getAffinityVoteTotal(Affinity affinity) const;

    /** @brief Clear all state. Call at run start and between runs. */
    void reset();

  private:
    std::map<std::string, RunCharacterState, std::less<>> m_states{};
    std::map<Affinity, float> m_fieldVoteTotals{};
};