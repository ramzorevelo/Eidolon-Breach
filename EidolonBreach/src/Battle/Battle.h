#pragma once
/**
 * @file Battle.h
 * @brief Orchestrates turn-based combat between two Parties.
 */

#include "Battle/BattleState.h"
#include "Battle/ITurnOrderCalculator.h"
#include "Battle/ResonanceField.h"
#include "Core/FieldDiscovery.h"
#include "Battle/TurnSlot.h"
#include "Entities/Party.h"
#include "UI/IInputHandler.h"
#include "UI/IRenderer.h"
#include <memory>
#include <vector>
#include "Core/BehaviorSignal.h"

class RunContext;
class EventBus;
class PlayableCharacter;
class IAction;
class SummonRegistry;
struct RunCharacterState;

class Battle
{
  public:
    /**
     * @brief Construct a battle.
     * @param playerParty    Player party reference.
     * @param enemyParty     Enemy party reference.
     * @param renderer       Rendering interface.
     * @param inputHandler   Input interface.
     * @param runContext     Per-run state for signal tracking and crystallization.
     * @param eventBus       Typed event bus for cross-system notifications.
     * @param turnOrderCalc  Turn order strategy; defaults to SpeedBasedTurnOrderCalculator.
     */
    Battle(Party &playerParty,
           Party &enemyParty,
           IRenderer &renderer,
           IInputHandler &inputHandler,
           RunContext &runContext,
           EventBus &eventBus,
           std::unique_ptr<ITurnOrderCalculator> turnOrderCalc = nullptr,
           const SummonRegistry *summonRegistry = nullptr);

    void run();

    [[nodiscard]] const ITurnOrderCalculator *getTurnOrderCalculator() const
    {
        return m_turnOrderCalc.get();
    }

  private:
    Party &m_playerParty;
    Party &m_enemyParty;
    RunContext &m_runContext;
    EventBus &m_eventBus;
    std::unique_ptr<ITurnOrderCalculator> m_turnOrderCalc;
    IRenderer &m_renderer;
    IInputHandler &m_inputHandler;
    ResonanceField m_field{};
    const SummonRegistry *m_summonRegistry{nullptr};

    void runBattleLoop(BattleState &state);
    bool isBattleOver() const;
    bool checkAndHandleBattleEnd(BattleState &state);

    void processPlayerTurn(Unit *unit, BattleState &state);
    void processEnemyTurn(Unit *unit, BattleState &state);

    void applyResonanceContribution(Unit &unit,
                                    Affinity actionAffinity,
                                    BattleState &state);
    void applyResonanceTrigger(Affinity affinity, BattleState &state);
    /**
     * @brief Apply the Exposure threshold 50 "Resonating" one-time bonus effect.
     *        Called from processPlayerTurn when the proc is armed and fires.
     */
    void applyResonatingProc(PlayableCharacter &pc,
                             const ActionResult &result,
                             BattleState &state);
    /**
     * @brief Apply the Exposure threshold 75 "Surging" one-time archetype bonus.
     *        Called from processPlayerTurn when the proc is armed and fires.
     */
    void applySurgingProc(PlayableCharacter &pc,
                          const ActionResult &result,
                          BattleState &state);

    /**
     * @brief Apply the immediate affinity burst when a character enters Breachborn.
     */
    void applyBreachbornEffect(PlayableCharacter &pc, BattleState &state);

    /**
     * @brief Apply Fracture-state per-turn effects at the start of a character's turn.
     *        Currently only implements Lyra's (Blaze) 5% self-DoT.
     */
    void applyFractureStartOfTurn(PlayableCharacter &pc, BattleState &state);

    /**
     * @brief Apply Breachborn active per-action bonus (Blaze: +50% damage + Burn).
     *        Called from processPlayerTurn when pc.isBreachbornActive() is true.
     */
    void applyBreachbornActionBonus(PlayableCharacter &pc,
                                    const ActionResult &result,
                                    BattleState &state);

    void processActionResult(PlayableCharacter &actor,
                             Party &allies,
                             const ActionResult &result,
                             BattleState &state);

    void checkExposureThresholds(PlayableCharacter &pc,
                                 int oldExposure,
                                 int newExposure,
                                 BattleState &state);

    void applyBehaviorSignals(PlayableCharacter &pc,
                              const IAction &action,
                              const ActionResult &result,
                              BattleState &state);
    void applyActionCategorySignals(PlayableCharacter &pc,
                                    const IAction &action,
                                    const ActionResult &result);
    void applyContextualSignals(PlayableCharacter &pc);
    void checkCrystallization(PlayableCharacter &pc, BattleState &state);

    void checkNewDeaths(const std::vector<bool> &aliveBefore,
                        const Party &party,
                        Unit *attacker,
                        BattleState &state);
    std::vector<bool> snapshotAliveStates(const Party &party) const;
    std::vector<bool> snapshotBreakStates(const Party &party) const;
    void processNewBreaks(const std::vector<bool> &before,
                          const Party &party,
                          Affinity actionAffinity,
                          BattleState &state);

    void resetAllPcConsumableState();
    void callVestigeOnBattleStart(BattleState &state);
    void callVestigeOnBattleEnd(BattleState &state);
    void collectDrops(BattleState &state);
    /**
     * @brief Spawn a Summon from the registry if the party cap allows.
     *        No-op when m_summonRegistry is nullptr or the id is not registered.
     * @param effect             The SummonEffect from the action result.
     * @param summonerContribution The acting PC's resonanceContribution at spawn time.
     */
    void processSummonEffect(const SummonEffect &effect,
                             int summonerContribution,
                             BattleState &state);

    /** @return Number of alive Summons currently in the player party. */
    [[nodiscard]] int countActiveSummons() const;

    /**
     * @brief Resolve and cache a stance ID for pc if the crystallization threshold
     *        is met and no stance is yet set. Called from checkCrystallization.
     * @return The dominant BehaviorSignal or BehaviorSignal::Aggressive as fallback.
     */
    [[nodiscard]] static BehaviorSignal findDominantSignal(
        const RunCharacterState &cs);
};