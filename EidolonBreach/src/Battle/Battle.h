#pragma once
/**
 * @file Battle.h
 * @brief Orchestrates turn-based combat between two Parties.
 */

#include "Battle/BattleState.h"
#include "Battle/ITurnOrderCalculator.h"
#include "Battle/ResonanceField.h"
#include "Battle/TurnSlot.h"
#include "Entities/Party.h"
#include "UI/IInputHandler.h"
#include "UI/IRenderer.h"
#include <memory>
#include <vector>

class RunContext;
class EventBus;
class PlayableCharacter;
class IAction;

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
           std::unique_ptr<ITurnOrderCalculator> turnOrderCalc = nullptr);

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

    void runBattleLoop(BattleState &state);
    bool isBattleOver() const;
    bool checkAndHandleBattleEnd(BattleState &state);

    void processPlayerTurn(Unit *unit, BattleState &state);
    void processEnemyTurn(Unit *unit, BattleState &state);

    void applyResonanceContribution(PlayableCharacter &pc,
                                    Affinity actionAffinity,
                                    BattleState &state);
    void applyResonanceTrigger(Affinity affinity, BattleState &state);

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
};