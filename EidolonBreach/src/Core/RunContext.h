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
#include <set>
#include <string>
#include <optional>
#include <string>
#include <string_view>


/**
 * @brief Game mode for the current run.
 *        Classic: earns character XP, pre-run loadout.
 *        Draft: no XP, Attune at Rest, separate RNG seed pool.
 */
enum class RunMode
{
    Classic,
    Draft,
};


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

    /** @brief Run mode for this run. Set by Dungeon::generate before any battles start. */
    RunMode runMode{RunMode::Classic};

    /**
     * @brief Active Field Discoveries for this run.
     *        Populated by Dungeon after each battle. Read by Battle::applyResonanceTrigger.
     */
    std::set<std::string> activeDiscoveries{};

    /**
     * @brief Check cumulative field vote totals and activate any newly met discoveries.
     *        Called by Dungeon::run after each battle completes.
     */
    void checkAndActivateDiscoveries();

  private:
    std::map<std::string, RunCharacterState, std::less<>> m_states{};
    std::map<Affinity, float> m_fieldVoteTotals{};
};