/**
 * @file Battle.cpp
 * @brief Battle orchestration implementation.
 */

#include "Battle/Battle.h"
#include "Battle/SpeedBasedTurnOrderCalculator.h"
#include "Core/BattleEvents.h"
#include "Core/CombatConstants.h"
#include "Core/EffectIds.h"
#include "Core/Effects/BurnEffect.h"
#include "Core/Effects/ShieldEffect.h"
#include "Core/Effects/SlowEffect.h"
#include "Core/EventBus.h"
#include "Core/RunContext.h"
#include "Entities/Enemy.h"
#include "Entities/PlayableCharacter.h"
#include "Items/Inventory.h"
#include <algorithm>

Battle::Battle(Party &playerParty,
               Party &enemyParty,
               IRenderer &renderer,
               IInputHandler &inputHandler,
               RunContext &runContext,
               EventBus &eventBus,
               std::unique_ptr<ITurnOrderCalculator> turnOrderCalc)
    : m_playerParty{playerParty},
      m_enemyParty{enemyParty},
      m_runContext{runContext},
      m_eventBus{eventBus},
      m_turnOrderCalc{turnOrderCalc ? std::move(turnOrderCalc)
                                    : std::make_unique<SpeedBasedTurnOrderCalculator>()},
      m_renderer{renderer},
      m_inputHandler{inputHandler}
{
}

void Battle::run()
{
    m_field.reset();
    BattleState state{0, 0, Affinity::Aether, m_field, m_inputHandler,
                      m_renderer, m_runContext, m_eventBus};
    state.playerParty = &m_playerParty;
    state.enemyParty = &m_enemyParty;

    m_eventBus.emit(BattleStartedEvent{&state});
    m_renderer.renderMessage("\n=== BATTLE START ===");

    runBattleLoop(state);

    const bool playerWon{m_enemyParty.isAllDead()};
    m_eventBus.emit(BattleEndedEvent{playerWon, &state});
    m_eventBus.clearBattleScope();
    resetAllPcConsumableState();
}

void Battle::runBattleLoop(BattleState &state)
{
    while (!isBattleOver())
    {
        auto turnOrder{m_turnOrderCalc->calculate(m_playerParty, m_enemyParty)};

        for (const auto &slot : turnOrder)
        {
            if (!slot.unit->isAlive() || isBattleOver())
                continue;

            m_renderer.renderPartyStatus(m_playerParty, m_enemyParty);

            // Snapshot both sides BEFORE ticking so DoT kills are detected correctly.
            const auto enemyAliveBefore{snapshotAliveStates(m_enemyParty)};
            const auto playerAliveBefore{snapshotAliveStates(m_playerParty)};

            for (const std::string &msg : slot.unit->tickEffects())
                m_renderer.renderMessage(msg);

            if (!slot.unit->isAlive())
            {
                checkNewDeaths(enemyAliveBefore, m_enemyParty, nullptr, state);
                checkNewDeaths(playerAliveBefore, m_playerParty, nullptr, state);
                if (checkAndHandleBattleEnd(state))
                    return;
                continue;
            }

            if (slot.isPlayer)
                processPlayerTurn(slot.unit, state);
            else
                processEnemyTurn(slot.unit, state);

            if (checkAndHandleBattleEnd(state))
                return;
        }
    }
}

bool Battle::isBattleOver() const
{
    return m_playerParty.isAllDead() || m_enemyParty.isAllDead();
}

bool Battle::checkAndHandleBattleEnd(BattleState &state)
{
    if (m_enemyParty.isAllDead())
    {
        collectDrops(state);
        for (std::size_t i{0}; i < m_enemyParty.size(); ++i)
        {
            const Unit *u{m_enemyParty.getUnitAt(i)};
            if (u && !u->isAlive())
                m_renderer.renderVictory(u->getName(), std::nullopt);
        }
        return true;
    }
    if (m_playerParty.isAllDead())
    {
        for (std::size_t i{0}; i < m_playerParty.size(); ++i)
        {
            const Unit *u{m_playerParty.getUnitAt(i)};
            if (u && !u->isAlive())
                m_renderer.renderDefeat(u->getName());
        }
        return true;
    }
    return false;
}

void Battle::processPlayerTurn(Unit *unit, BattleState &state)
{
    if (!unit)
        return;

    auto *pc{dynamic_cast<PlayableCharacter *>(unit)};
    if (pc)
    {
        pc->tickArchSkillCooldown();
        pc->tickConsumableCooldown();
    }

    auto enemyAliveBefore{snapshotAliveStates(m_enemyParty)};
    auto enemyBreaksBefore{snapshotBreakStates(m_enemyParty)};

    const ActionResult result{unit->takeTurn(m_playerParty, m_enemyParty, state)};
    m_renderer.renderActionResult(unit->getName(), result);

    processNewBreaks(enemyBreaksBefore, m_enemyParty, result.actionAffinity, state);
    checkNewDeaths(enemyAliveBefore, m_enemyParty, unit, state);

    if (pc)
    {
        applyResonanceContribution(*pc, result.actionAffinity, state);
        processActionResult(*pc, m_playerParty, result, state);

        // Determine the action that was used from the action result metadata.
        const auto &abilities{pc->getAbilities()};
        for (const auto &ability : abilities)
        {
            if (ability->getAffinity() == result.actionAffinity)
            {
                applyBehaviorSignals(*pc, *ability, result, state);
                break;
            }
        }
    }

    if (state.resonanceField.isReady())
    {
        const Affinity triggered{state.resonanceField.trigger()};
        applyResonanceTrigger(triggered, state);
        m_renderer.renderMessage(">> Resonance Field: " +
                                 affinityToString(triggered) + " triggered! <<");
    }
    m_renderer.renderResonanceField(state.resonanceField);
}

void Battle::processEnemyTurn(Unit *unit, BattleState &state)
{
    auto playerAliveBefore{snapshotAliveStates(m_playerParty)};
    const ActionResult result{unit->takeTurn(m_enemyParty, m_playerParty, state)};

    if (result.type == ActionResult::Type::Skip)
        m_renderer.renderStunned(unit->getName());
    else
        m_renderer.renderActionResult(unit->getName(), result);

    checkNewDeaths(playerAliveBefore, m_playerParty, unit, state);
}

void Battle::applyResonanceContribution(PlayableCharacter &pc,
                                        Affinity actionAffinity,
                                        BattleState &state)
{
    int contribution{pc.getResonanceContribution()};
    if (actionAffinity == state.floorAffinity)
    {
        contribution = static_cast<int>(
            static_cast<float>(contribution) *
            (1.0f + CombatConstants::kFloorAffinityResonanceBonus));
    }
    state.resonanceField.addContribution(actionAffinity, contribution);
    m_runContext.recordFieldVotes(actionAffinity,
                                  state.resonanceField.getVotes(actionAffinity));
}

void Battle::applyResonanceTrigger(Affinity affinity, BattleState &state)
{
    state.eventBus.emit(ResonanceFieldTriggeredEvent{affinity, &state});

    switch (affinity)
    {
    case Affinity::Blaze:
        for (Unit *u : m_enemyParty.getAliveUnits())
            u->applyEffect(std::make_unique<BurnEffect>(10, 2));
        break;
    case Affinity::Frost:
        for (Unit *u : m_enemyParty.getAliveUnits())
            u->applyEffect(std::make_unique<SlowEffect>(0.30f, 2));
        break;
    case Affinity::Tempest:
        for (Unit *u : m_playerParty.getAliveUnits())
            if (auto *pc = dynamic_cast<PlayableCharacter *>(u))
                pc->gainEnergy(20);
        break;
    case Affinity::Terra:
        for (Unit *u : m_playerParty.getAliveUnits())
            u->applyEffect(std::make_unique<ShieldEffect>(30, 2));
        break;
    case Affinity::Aether:
        for (Unit *u : m_playerParty.getAliveUnits())
            u->removeEffectsByTag(EffectTags::kDebuff);
        for (Unit *u : m_enemyParty.getAliveUnits())
            u->removeEffectsByTag(EffectTags::kBuff);
        break;
    }
}

void Battle::processActionResult(PlayableCharacter &actor,
                                 Party &allies,
                                 const ActionResult &result,
                                 BattleState &state)
{
    if (result.spGained != 0)
        allies.gainSp(result.spGained);

    if (result.exposureDelta != 0)
    {
        const int oldExposure{actor.getExposure()};
        actor.modifyExposure(result.exposureDelta);
        checkExposureThresholds(actor, oldExposure, actor.getExposure(), state);
    }
}

void Battle::checkExposureThresholds(PlayableCharacter &pc,
                                     int oldExposure,
                                     int newExposure,
                                     BattleState &state)
{
    for (int threshold : {CombatConstants::kExposureThreshold50,
                          CombatConstants::kExposureThreshold75,
                          CombatConstants::kExposureThreshold100})
    {
        if (oldExposure < threshold && newExposure >= threshold)
            state.eventBus.emit(ExposureThresholdEvent{&pc, threshold, &state});
    }
}

void Battle::applyBehaviorSignals(PlayableCharacter &pc,
                                  const IAction &action,
                                  const ActionResult &result,
                                  BattleState &state)
{
    applyActionCategorySignals(pc, action, result);
    applyContextualSignals(pc);
    checkCrystallization(pc, state);
}

void Battle::applyActionCategorySignals(PlayableCharacter &pc,
                                        const IAction &action,
                                        const ActionResult &result)
{
    const ActionData &data{action.getActionData()};
    RunCharacterState &cs{m_runContext.getCharacterState(pc.getId())};

    if (data.category == ActionCategory::Basic ||
        data.category == ActionCategory::ArchSkill)
        ++cs.signalCounts[BehaviorSignal::Aggressive];

    if (data.category == ActionCategory::Slot)
        ++cs.signalCounts[BehaviorSignal::Methodical];

    if (data.category == ActionCategory::Vent)
        ++cs.signalCounts[BehaviorSignal::Reactive];

    if (data.category == ActionCategory::Consumable &&
        result.type == ActionResult::Type::Heal)
        ++cs.signalCounts[BehaviorSignal::Supportive];

    if (result.spGained > 0)
    {
        cs.totalSpGenerated += result.spGained;
        const int surplus{cs.totalSpGenerated - cs.totalSpSpent};
        if (surplus >= CombatConstants::kSupportiveSpSurplusThreshold)
            ++cs.signalCounts[BehaviorSignal::Supportive];
    }
}

void Battle::applyContextualSignals(PlayableCharacter &pc)
{
    RunCharacterState &cs{m_runContext.getCharacterState(pc.getId())};
    const bool highExposure{pc.getExposure() >= CombatConstants::kExposureThreshold50};
    const int thirtyPctHp{pc.getFinalStats().maxHp * 30 / 100};
    const bool lowHp{pc.getHp() <= thirtyPctHp};

    if (highExposure || lowHp)
        ++cs.signalCounts[BehaviorSignal::Sacrificial];

    if (pc.getExposure() < 40)
    {
        ++cs.consecutiveLowExposureTurns;
        if (cs.consecutiveLowExposureTurns >= 3)
            ++cs.signalCounts[BehaviorSignal::Reactive];
    }
    else
    {
        cs.consecutiveLowExposureTurns = 0;
    }
}

void Battle::checkCrystallization(PlayableCharacter &pc, BattleState &state)
{
    RunCharacterState &cs{m_runContext.getCharacterState(pc.getId())};
    if (cs.crystallizedStanceId.has_value())
        return;

    int dominantCount{0};
    for (const auto &[signal, count] : cs.signalCounts)
        dominantCount = std::max(dominantCount, count);

    const float ratio{static_cast<float>(dominantCount) /
                      static_cast<float>(CombatConstants::kCrystallizationThreshold)};
    cs.synchronicityProgress = std::min(100, static_cast<int>(ratio * 100.0f));

    if (dominantCount >= CombatConstants::kCrystallizationThreshold)
    {
        cs.crystallizedStanceId = "";
        cs.synchronicityProgress = 0;
        state.eventBus.emit(StanceCrystallizedEvent{&pc, ""});
    }
}

std::vector<bool> Battle::snapshotAliveStates(const Party &party) const
{
    std::vector<bool> states{};
    states.reserve(party.size());
    for (std::size_t i{0}; i < party.size(); ++i)
    {
        const Unit *u{party.getUnitAt(i)};
        states.push_back(u != nullptr && u->isAlive());
    }
    return states;
}

std::vector<bool> Battle::snapshotBreakStates(const Party &party) const
{
    std::vector<bool> states{};
    states.reserve(party.size());
    for (std::size_t i{0}; i < party.size(); ++i)
    {
        const Unit *u{party.getUnitAt(i)};
        states.push_back(u != nullptr && u->isBroken());
    }
    return states;
}

void Battle::checkNewDeaths(const std::vector<bool> &aliveBefore,
                            const Party &party,
                            Unit *attacker,
                            BattleState &state)
{
    for (std::size_t i{0}; i < party.size() && i < aliveBefore.size(); ++i)
    {
        Unit *u{const_cast<Unit *>(party.getUnitAt(i))};
        if (!u || !aliveBefore[i] || u->isAlive())
            continue;
        state.eventBus.emit(UnitDefeatedEvent{u, attacker, &state});
    }
}

void Battle::processNewBreaks(const std::vector<bool> &before,
                              const Party &party,
                              Affinity actionAffinity,
                              BattleState &state)
{
    for (std::size_t i{0}; i < party.size() && i < before.size(); ++i)
    {
        Unit *u{const_cast<Unit *>(party.getUnitAt(i))};
        if (!u || before[i] || !u->isBroken())
            continue;

        m_renderer.renderBreak(u->getName());
        state.eventBus.emit(BreakTriggeredEvent{
            dynamic_cast<Enemy *>(u), actionAffinity, &state});

        if (auto *e{dynamic_cast<Enemy *>(u)})
        {
            const BreakEffect &effect{e->getBreakEffect()};
            if (effect.onBreak)
                effect.onBreak(*e, state);
        }
    }
}

void Battle::resetAllPcConsumableState()
{
    for (std::size_t i{0}; i < m_playerParty.size(); ++i)
    {
        Unit *u{m_playerParty.getUnitAt(i)};
        if (auto *pc = dynamic_cast<PlayableCharacter *>(u))
        {
            pc->resetBattleConsumableState();
            pc->resetArchSkillCooldown();
        }
    }
}

void Battle::collectDrops(BattleState &state)
{
    const unsigned int seed{static_cast<unsigned int>(state.turnNumber)};
    for (std::size_t i{0}; i < m_enemyParty.size(); ++i)
    {
        Unit *u{m_enemyParty.getUnitAt(i)};
        if (!u || u->isAlive())
            continue;
        if (auto *e = dynamic_cast<Enemy *>(u))
        {
            for (const Drop &drop : e->generateDrops(seed))
                if (drop.type == Drop::Type::Gold ||
                    drop.type == Drop::Type::GuaranteedItem)
                    m_playerParty.getInventory().gold += drop.goldAmount;
        }
    }
}