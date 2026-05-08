/**
 * @file Battle.cpp
 * @brief Battle orchestration implementation.
 */
#include <set>
#include "Core/FieldDiscovery.h"
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
#include "Entities/Summon.h"
#include "Summons/SummonRegistry.h"
#include "Battle/StanceModifiers.h"
#include "Characters/Lyra/Lyra.h"

Battle::Battle(Party &playerParty,
               Party &enemyParty,
               IRenderer &renderer,
               IInputHandler &inputHandler,
               RunContext &runContext,
               EventBus &eventBus,
               std::unique_ptr<ITurnOrderCalculator> turnOrderCalc,
               const SummonRegistry *summonRegistry)
    : m_playerParty{playerParty},
      m_enemyParty{enemyParty},
      m_runContext{runContext},
      m_eventBus{eventBus},
      m_turnOrderCalc{turnOrderCalc ? std::move(turnOrderCalc)
                                    : std::make_unique<SpeedBasedTurnOrderCalculator>()},
      m_renderer{renderer},
      m_inputHandler{inputHandler},
      m_summonRegistry{summonRegistry}
{
}

void Battle::run()
{
    m_field.reset();
    BattleState state{0, 0, Affinity::Aether, m_field, m_inputHandler,
                      m_renderer, m_runContext, m_eventBus};
    state.playerParty = &m_playerParty;
    state.enemyParty = &m_enemyParty;
    m_eventBus.subscribe<ExposureThresholdEvent>(
        [this, &state](const ExposureThresholdEvent &e)
        {
            if (!e.character)
                return;
            if (e.threshold == CombatConstants::kExposureThreshold50)
            {
                e.character->armResonatingProc();
                state.renderer.renderMessage(
                    e.character->getName() + " begins Resonating! (proc armed)");
            }
            else if (e.threshold == CombatConstants::kExposureThreshold75)
            {
                e.character->armSurgingProc();
                state.renderer.renderMessage(
                    e.character->getName() + " is Surging! (next action boosted)");
            }
            else if (e.threshold == CombatConstants::kExposureThreshold100)
            {
                e.character->activateBreachborn();
                applyBreachbornEffect(*e.character, state);
                state.renderer.renderMessage(
                    e.character->getName() + " enters BREACHBORN!");
            }
        },
        EventScope::Battle);

    m_eventBus.emit(BattleStartedEvent{&state});
    callVestigeOnBattleStart(state);
    m_renderer.renderMessage("\n=== BATTLE START ===");

    runBattleLoop(state);

    const bool playerWon{m_enemyParty.isAllDead()};
    m_eventBus.emit(BattleEndedEvent{playerWon, &state});
    callVestigeOnBattleEnd(state);
    m_eventBus.clearBattleScope();
    resetAllPcConsumableState();
}

void Battle::runBattleLoop(BattleState &state)
{
    while (!isBattleOver())
    {
        auto turnOrder{m_turnOrderCalc->calculate(m_playerParty, m_enemyParty)};
        m_renderer.renderTurnOrder(turnOrder);

        for (std::size_t i{0}; i < m_enemyParty.size(); ++i)
        {
            const Unit *u{m_enemyParty.getUnitAt(i)};
            if (!u || !u->isAlive())
                continue;
            const std::string intent{u->getIntentLabel()};
            if (!intent.empty())
                m_renderer.renderMessage(u->getName() + " | " + intent);
        }

        for (std::size_t slotIdx = 0; slotIdx < turnOrder.size(); ++slotIdx)
        {
            const TurnSlot &slot = turnOrder[slotIdx];
            if (!slot.unit || !slot.unit->isAlive() || isBattleOver())
                continue;

            m_renderer.renderPartyStatus(m_playerParty, m_enemyParty);

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

            // Include any units summoned during this turn in the round.
            for (std::size_t i{0}; i < m_playerParty.size(); ++i)
            {
                Unit *u{m_playerParty.getUnitAt(i)};
                if (!u || !u->isAlive())
                    continue;
                const bool already{std::any_of(
                    turnOrder.begin(), turnOrder.end(),
                    [u](const TurnSlot &s)
                    { return s.unit == u; })};
                if (!already)
                    turnOrder.push_back({u, true, i});
            }

            if (slot.unit->isSummon())
            {
                auto *s{dynamic_cast<Summon *>(slot.unit)};
                if (s)
                {
                    s->tickDuration();
                    if (s->isExpired())
                    {
                        const std::string name{slot.unit->getName()};
                        const std::string id{slot.unit->getId()};
                        m_renderer.renderMessage(name + " fades away.");
                        m_playerParty.removeUnit(id);
                    }
                }
            }

            if (checkAndHandleBattleEnd(state))
                return;

            // Update strip to show only remaining units this round.
            const std::vector<TurnSlot> remaining{
                turnOrder.begin() + static_cast<std::ptrdiff_t>(slotIdx) + 1,
                turnOrder.end()};
            m_renderer.renderTurnOrder(remaining);

            m_renderer.presentPause(slot.isPlayer ? 180 : 350);
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
        return true;
    }
    if (m_playerParty.isAllDead())
        return true;
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

        // Fracture start-of-turn effect (permanent after Breachborn ends).
        if (pc->isFractured())
            applyFractureStartOfTurn(*pc, state);

        // Breachborn turn counter.
        if (pc->isBreachbornActive())
        {
            m_renderer.renderMessage(pc->getName() +
                                     " — BREACHBORN (" +
                                     std::to_string(pc->getBreachbornTurnsRemaining()) +
                                     " turns remaining)");
            const bool fractureActivated{pc->tickBreachborn()};
            if (fractureActivated)
                m_renderer.renderMessage(pc->getName() +
                                         " — Breachborn ends. FRACTURE activated!");
        }

        for (auto &v : m_playerParty.getVestiges())
            v->onTurnStart(*pc, state);
    }

    auto enemyAliveBefore{snapshotAliveStates(m_enemyParty)};
    auto enemyBreaksBefore{snapshotBreakStates(m_enemyParty)};

    // result is non-const so vestige onAction hooks can modify it.
    ActionResult result{unit->takeTurn(m_playerParty, m_enemyParty, state)};
    state.renderer.clearTargetHighlight(); 

    if (result.targetEnemyIndex >= 0 && state.enemyParty != nullptr)
    {
        const Unit *t{state.enemyParty->getUnitAt(
            static_cast<std::size_t>(result.targetEnemyIndex))};
        if (t)
            result.targetName = t->getName();
    }

    if (pc)
    {
        for (auto &v : m_playerParty.getVestiges())
            v->onAction(*pc, result, state);
    }

    m_renderer.renderActionResult(unit->getName(), result);

    processNewBreaks(enemyBreaksBefore, m_enemyParty, result.actionAffinity, state);
    checkNewDeaths(enemyAliveBefore, m_enemyParty, unit, state);

    // Resonance contribution applies to all player-side units (PCs and Summons).
    applyResonanceContribution(*unit, result.actionAffinity, state);

    if (pc)
    {
        processActionResult(*pc, m_playerParty, result, state);
        if (pc->isResonatingProcArmed() &&
            result.actionAffinity == pc->getAffinity())
        {
            pc->consumeResonatingProc();
            applyResonatingProc(*pc, result, state);
        }

        if (pc->isSurgingProcArmed())
        {
            pc->consumeSurgingProc();
            applySurgingProc(*pc, result, state);
        }

        if (result.ventConsolation && pc->isResonatingProcArmed())
        {
            state.renderer.renderMessage(
                pc->getName() + " vents — Resonating discharge fires as consolation!");
            pc->consumeResonatingProc();
            applyResonatingProc(*pc, result, state);
        }

        if (pc && pc->isBreachbornActive())
            applyBreachbornActionBonus(*pc, result, state);

        if (result.summonEffect.has_value())
        {
            processSummonEffect(*result.summonEffect, pc->getResonanceContribution(), state);
        }

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

void Battle::applyResonanceContribution(Unit &unit,
                                        Affinity actionAffinity,
                                        BattleState &state)
{
    int contribution{unit.getResonanceContribution()};
    if (contribution == 0)
        return;

    if (actionAffinity == state.floorAffinity)
    {
        contribution = static_cast<int>(
            static_cast<float>(contribution) *
            (1.0f + CombatConstants::kFloorAffinityResonanceBonus));
    }

    // Apply Stance modifier if the unit is a PC with a crystallized Stance.
    if (const auto *pc{dynamic_cast<const PlayableCharacter *>(&unit)})
    {
        const RunCharacterState *cs{m_runContext.findCharacterState(pc->getId())};
        if (cs && cs->crystallizedStanceId.has_value() &&
            !cs->crystallizedStanceId->empty())
        {
            contribution = StanceModifiers::applyResonanceModifier(
                *cs->crystallizedStanceId, *pc, actionAffinity, contribution, state);
        }
    }

    state.resonanceField.addContribution(actionAffinity, contribution);
    m_runContext.recordFieldVotes(actionAffinity,
                                  state.resonanceField.getVotes(actionAffinity));
}

void Battle::applyResonanceTrigger(Affinity affinity, BattleState &state)
{
    state.eventBus.emit(ResonanceFieldTriggeredEvent{affinity, &state});
    const std::set<std::string> &discoveries{state.runContext.activeDiscoveries};

    switch (affinity)
    {
    case Affinity::Blaze:
        for (Unit *u : m_enemyParty.getAliveUnits())
            u->applyEffect(std::make_unique<BurnEffect>(10, 2));
        // Molten Lattice: also shield all allies.
        if (discoveries.count(std::string{FieldDiscoveryIds::kMoltenLattice}))
        {
            for (Unit *u : m_playerParty.getAliveUnits())
                u->applyEffect(std::make_unique<ShieldEffect>(15, 1));
            m_renderer.renderMessage(">> Molten Lattice: allies shielded! <<");
        }
        break;

    case Affinity::Frost:
    {
        // Arctic Surge: apply Slow for 3 turns instead of 2.
        const int slowDuration{
            discoveries.count(std::string{FieldDiscoveryIds::kArcticSurge}) ? 3 : 2};
        for (Unit *u : m_enemyParty.getAliveUnits())
            u->applyEffect(std::make_unique<SlowEffect>(0.30f, slowDuration));
        if (slowDuration == 3)
            m_renderer.renderMessage(">> Arctic Surge: extended Slow! <<");
        break;
    }

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
        // Lattice Attunement: add extra Aether votes to field.
        if (discoveries.count(std::string{FieldDiscoveryIds::kLatticeAttunement}))
        {
            state.resonanceField.addContribution(Affinity::Aether,
                                                 static_cast<int>(
                                                     ResonanceField::kAetherVoteFraction * 10.0f));
            m_renderer.renderMessage(">> Lattice Attunement: Aether echo! <<");
        }
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
        const BehaviorSignal dominant{findDominantSignal(cs)};
        const std::string resolvedId{
            StanceModifiers::resolveStanceId(pc.getId(), dominant)};

        cs.crystallizedStanceId = resolvedId;
        cs.synchronicityProgress = 0;
        state.eventBus.emit(StanceCrystallizedEvent{&pc, resolvedId});

        if (!resolvedId.empty())
            state.renderer.renderMessage(
                pc.getName() + " crystallizes stance: " + resolvedId);
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
        if (&party == &m_enemyParty)
            state.renderer.renderVictory(u->getName(), std::nullopt);
        else
            state.renderer.renderDefeat(u->getName());
        state.renderer.renderPartyStatus(m_playerParty, m_enemyParty);
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

        if (!u->isAlive())
            continue; // death message takes priority; skip break notification
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

void Battle::callVestigeOnBattleStart(BattleState &state)
{
    for (auto &v : m_playerParty.getVestiges())
        v->onBattleStart(*this, state);
}

void Battle::callVestigeOnBattleEnd(BattleState &state)
{
    for (auto &v : m_playerParty.getVestiges())
        v->onBattleEnd(state);
}

void Battle::processSummonEffect(const SummonEffect &effect,
                                 int summonerContribution,
                                 BattleState &state)
{
    if (!m_summonRegistry)
        return;

    if (countActiveSummons() >= CombatConstants::kMaxActiveSummons)
    {
        state.renderer.renderMessage("No room for another Manifestation — limit reached.");
        return;
    }

    const SummonDefinition *def{m_summonRegistry->find(effect.summonId)};
    if (!def)
    {
        state.renderer.renderMessage("Unknown summon id: " + effect.summonId);
        return;
    }

    auto summon{std::make_unique<Summon>(*def, summonerContribution, effect.summonerAtk)};
    state.renderer.renderMessage(def->displayName + " manifests!");
    m_playerParty.addUnit(std::move(summon));
}

BehaviorSignal Battle::findDominantSignal(const RunCharacterState &cs)
{
    BehaviorSignal dominant{BehaviorSignal::Aggressive};
    int highestCount{0};
    for (const auto &[signal, count] : cs.signalCounts)
    {
        if (count > highestCount)
        {
            highestCount = count;
            dominant = signal;
        }
    }
    return dominant;
}

int Battle::countActiveSummons() const
{
    int count{0};
    for (std::size_t i{0}; i < m_playerParty.size(); ++i)
    {
        const Unit *u{m_playerParty.getUnitAt(i)};
        if (u && u->isAlive() && u->isSummon())
            ++count;
    }
    return count;
}

void Battle::applyResonatingProc(PlayableCharacter &pc,
                                 const ActionResult &result,
                                 BattleState &state)
{
    switch (pc.getAffinity())
    {
    case Affinity::Blaze:
        if (result.targetEnemyIndex >= 0 && state.enemyParty != nullptr)
        {
            Unit *t{state.enemyParty->getUnitAt(
                static_cast<std::size_t>(result.targetEnemyIndex))};
            if (t && t->isAlive())
                t->applyEffect(std::make_unique<BurnEffect>(5, 2));
        }
        break;

    case Affinity::Frost:
        if (result.targetEnemyIndex >= 0 && state.enemyParty != nullptr)
        {
            Unit *t{state.enemyParty->getUnitAt(
                static_cast<std::size_t>(result.targetEnemyIndex))};
            if (t && t->isAlive())
                t->applyEffect(std::make_unique<SlowEffect>(0.20f, 2));
        }
        break;

    case Affinity::Tempest:
        pc.gainEnergy(15);
        break;

    case Affinity::Terra:
    {
        const int shield{
            std::max(1, static_cast<int>(
                            static_cast<float>(pc.getFinalStats().maxHp) * 0.10f))};
        pc.applyEffect(std::make_unique<ShieldEffect>(shield, 2));
        break;
    }

    case Affinity::Aether:
        state.resonanceField.addContribution(
            Affinity::Aether,
            std::max(1, pc.getResonanceContribution() / 2));
        break;
    }

    state.renderer.renderMessage(
        pc.getName() + " — Resonating proc fires!");
}

void Battle::applySurgingProc(PlayableCharacter &pc,
                              const ActionResult &result,
                              BattleState &state)
{
    const std::string &arch{pc.getArchetype()};

    if (arch == "Striker")
    {
        // +25% of the damage dealt as bonus true damage to the primary target.
        if (result.value > 0 && result.targetEnemyIndex >= 0 &&
            state.enemyParty != nullptr)
        {
            Unit *t{state.enemyParty->getUnitAt(
                static_cast<std::size_t>(result.targetEnemyIndex))};
            if (t && t->isAlive())
            {
                const int bonus{std::max(1, result.value / 4)};
                t->takeTrueDamage(bonus);
                state.renderer.renderMessage(
                    pc.getName() + " — Surging: +" + std::to_string(bonus) +
                    " true damage!");
            }
        }
    }
    else if (arch == "Conduit")
    {
        if (state.playerParty != nullptr)
            state.playerParty->gainSp(20);
        state.renderer.renderMessage(pc.getName() + " — Surging: +20 SP!");
    }
    else if (arch == "Weaver")
    {
        if (state.enemyParty != nullptr)
        {
            for (Unit *u : state.enemyParty->getAliveUnits())
                u->extendEffectsByTag(EffectTags::kDebuff, 1);
        }
        state.renderer.renderMessage(
            pc.getName() + " — Surging: enemy debuffs extended by 1 turn!");
    }
    else if (arch == "Anchor")
    {
        const int shield{
            std::max(1, static_cast<int>(
                            static_cast<float>(pc.getFinalStats().maxHp) * 0.15f))};
        pc.applyEffect(std::make_unique<ShieldEffect>(shield, 2));
        state.renderer.renderMessage(
            pc.getName() + " — Surging: shield +" + std::to_string(shield) + "!");
    }
    else
    {
        state.renderer.renderMessage(pc.getName() + " — Surging!");
    }
}

void Battle::applyBreachbornEffect(PlayableCharacter &pc, BattleState &state)
{
    switch (pc.getAffinity())
    {
    case Affinity::Blaze:
        if (state.enemyParty != nullptr)
            for (Unit *u : state.enemyParty->getAliveUnits())
                u->applyEffect(std::make_unique<BurnEffect>(10, 3));
        state.renderer.renderMessage(
            pc.getName() + " — Breachborn: Burn descends on all enemies!");
        break;

    case Affinity::Frost:
        if (state.enemyParty != nullptr)
            for (Unit *u : state.enemyParty->getAliveUnits())
                u->applyEffect(std::make_unique<SlowEffect>(0.50f, 3));
        state.renderer.renderMessage(
            pc.getName() + " — Breachborn: Arctic storm slows all enemies!");
        break;

    case Affinity::Tempest:
        if (state.playerParty != nullptr)
            for (Unit *u : state.playerParty->getAliveUnits())
                if (auto *ally = dynamic_cast<PlayableCharacter *>(u))
                    ally->gainEnergy(20);
        state.renderer.renderMessage(
            pc.getName() + " — Breachborn: Tempest surges through the party (+20 Energy)!");
        break;

    case Affinity::Terra:
        if (state.playerParty != nullptr)
            for (Unit *u : state.playerParty->getAliveUnits())
                u->applyEffect(std::make_unique<ShieldEffect>(25, 3));
        state.renderer.renderMessage(
            pc.getName() + " — Breachborn: Terra fortress shields the party!");
        break;

    case Affinity::Aether:
        state.resonanceField.addContribution(Affinity::Aether, 30);
        state.renderer.renderMessage(
            pc.getName() + " — Breachborn: Aether floods the Resonance Field!");
        break;
    }
}

void Battle::applyFractureStartOfTurn(PlayableCharacter &pc, BattleState &state)
{
    if (pc.getAffinity() == Affinity::Blaze)
    {
        // Lyra Fracture: 5% maxHP self-DoT per her turn.
        const int selfDamage{
            std::max(1, static_cast<int>(
                            static_cast<float>(pc.getFinalStats().maxHp) * 0.05f))};
        pc.takeTrueDamage(selfDamage);
        state.renderer.renderMessage(
            pc.getName() + " — Fracture: -" + std::to_string(selfDamage) +
            " HP (self-DoT)");
    }
    else
    {
        // Other Fracture effects deferred to IPassiveTrait (v0.9.x).
        state.renderer.renderMessage(pc.getName() + " — [Fractured]");
    }
}

void Battle::applyBreachbornActionBonus(PlayableCharacter &pc,
                                        const ActionResult &result,
                                        BattleState &state)
{
    if (pc.getAffinity() != Affinity::Blaze)
        return; // Placeholder: only Blaze has per-action Breachborn bonus for now.

    // +50% bonus damage as true damage to primary target.
    if (result.value > 0 && result.targetEnemyIndex >= 0 &&
        state.enemyParty != nullptr)
    {
        Unit *t{state.enemyParty->getUnitAt(
            static_cast<std::size_t>(result.targetEnemyIndex))};
        if (t && t->isAlive())
        {
            const int bonus{result.value / 2};
            t->takeTrueDamage(bonus);
            // Also re-apply Burn on every Breachborn action.
            t->applyEffect(std::make_unique<BurnEffect>(5, 1));
            state.renderer.renderMessage(
                pc.getName() + " — Breachborn Blaze: +" +
                std::to_string(bonus) + " bonus damage + Burn!");
        }
    }
}