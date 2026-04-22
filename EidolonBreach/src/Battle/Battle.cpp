#include "Battle/Battle.h"
#include "Battle/SpeedBasedTurnOrderCalculator.h"
#include "Core/Effects/BurnEffect.h"
#include "Core/Effects/ShieldEffect.h"
#include "Core/Effects/SlowEffect.h"
#include "Core/EffectIds.h"
#include "Entities/PlayableCharacter.h"
#include "Core/ActionResult.h"
#include "Items/Inventory.h"
#include "Entities/Enemy.h"
#include "UI/ConsoleRenderer.h"
#include <algorithm>
#include <iostream>

Battle::Battle(Party &playerParty,
               Party &enemyParty,
               IRenderer &renderer,
               IInputHandler &inputHandler,
               std::unique_ptr<ITurnOrderCalculator> turnOrderCalc)
    : m_playerParty{playerParty}, m_enemyParty{enemyParty}, m_renderer{renderer}, m_inputHandler{inputHandler}, m_turnOrderCalc{turnOrderCalc ? std::move(turnOrderCalc)
                                                                                                                                              : std::make_unique<SpeedBasedTurnOrderCalculator>()}
{
}

std::vector<bool> Battle::snapshotBreakStates(const Party &party) const
{
    std::vector<bool> states{};
    states.reserve(party.size());
    for (std::size_t i{0}; i < party.size(); ++i)
    {
        const Unit *u{party.getUnitAt(i)};
        states.push_back(u ? u->isBroken() : false);
    }
    return states;
}

void Battle::processNewBreaks(const std::vector<bool> &before,
                              const Party &party,
                              BattleState &state)
{
    for (std::size_t i{0}; i < party.size() && i < before.size(); ++i)
    {
        Unit *u{const_cast<Unit *>(party.getUnitAt(i))};
        if (!u || before[i] || !u->isBroken())
            continue;

        m_renderer.renderBreak(u->getName());

        // Fire the BreakEffect callback if one is registered on this enemy.
        // dynamic_cast is permitted in Battle (UI/integration layer) per existing precedent.
        if (auto *e{dynamic_cast<Enemy *>(u)})
        {
            const BreakEffect &effect{e->getBreakEffect()};
            if (effect.onBreak)
                effect.onBreak(*e, state);
        }
    }
}

bool Battle::isBattleOver() const
{
    return m_playerParty.isAllDead() || m_enemyParty.isAllDead();
}

void Battle::processPlayerTurn(Unit *unit, BattleState &state)
{
    if (!unit)
        return;

    // FIXME Phase 6: replace dynamic_cast with Unit::onTurnStart() virtual hook.
    if (auto *pc = dynamic_cast<PlayableCharacter *>(unit))
    {
        pc->tickArchSkillCooldown();
        pc->tickConsumableCooldown();
    }

    auto breaksBefore{snapshotBreakStates(m_enemyParty)};
    ActionResult result{unit->takeTurn(m_playerParty, m_enemyParty, state)};
    m_renderer.renderActionResult(unit->getName(), result);
    processNewBreaks(breaksBefore, m_enemyParty, state);

    if (auto *pc = dynamic_cast<PlayableCharacter *>(unit))
        processActionResult(*pc, m_playerParty, result);

    if (state.resonanceField.isReady())
    {
        Affinity triggered{state.resonanceField.trigger()};
        applyResonanceTrigger(triggered);
        m_renderer.renderMessage(">> Resonance Field: " +
                                 affinityToString(triggered) + " triggered! <<");
    }
    m_renderer.renderResonanceField(state.resonanceField);
}

void Battle::applyResonanceTrigger(Affinity affinity)
{
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
        // Dynamic cast permitted here: no generic momentum interface on Unit
        // until Phase 9 (IPassiveTrait). Flagged for Phase 9 revisit.
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


void Battle::processEnemyTurn(Unit *unit, BattleState &state)
{
    ActionResult result{unit->takeTurn(m_enemyParty, m_playerParty, state)};
    if (result.type == ActionResult::Type::Skip)
    {
        m_renderer.renderStunned(unit->getName());
    }
    else
    {
        m_renderer.renderActionResult(unit->getName(), result);
    }
}

bool Battle::checkAndHandleBattleEnd(BattleState &state)
{
    if (m_enemyParty.isAllDead())
    {
        collectDrops(state);
        for (std::size_t i{0}; i < m_enemyParty.size(); ++i)
        {
            Unit *u{m_enemyParty.getUnitAt(i)};
            if (u && !u->isAlive())
            {
                if (auto *e = dynamic_cast<Enemy *>(u))
                    m_renderer.renderVictory(e->getName(), std::nullopt);
            }
        }
        resetAllPcConsumableState();
        return true;
    }

    if (m_playerParty.isAllDead())
    {
        for (std::size_t i{0}; i < m_playerParty.size(); ++i)
        {
            Unit *u{m_playerParty.getUnitAt(i)};
            if (u && !u->isAlive())
                m_renderer.renderDefeat(u->getName());
        }
        resetAllPcConsumableState();
        return true;
    }
    return false;
}


void Battle::resetAllPcConsumableState()
{
    for (std::size_t i{0}; i < m_playerParty.size(); ++i)
    {
        Unit *u{m_playerParty.getUnitAt(i)};
        if (auto *pc = dynamic_cast<PlayableCharacter *>(u))
            pc->resetBattleConsumableState();
    }
}

void Battle::run()
{
    m_renderer.renderMessage("\n=== BATTLE START ===");
    m_field.reset();
    BattleState state{0, 0, m_field, m_inputHandler, m_renderer};
    state.playerParty = &m_playerParty;
    state.enemyParty = &m_enemyParty;

    while (!isBattleOver())
    {
        m_renderer.renderPartyStatus(m_playerParty, m_enemyParty);
        auto turnOrder{m_turnOrderCalc->calculate(m_playerParty, m_enemyParty)};

        for (const auto &slot : turnOrder)
        {
            if (!slot.unit->isAlive())
                continue;
            if (isBattleOver())
                break;

            for (const std::string &msg : slot.unit->tickEffects())
                m_renderer.renderMessage(msg);

            if (!slot.unit->isAlive())
            {
                if ((checkAndHandleBattleEnd(state)))
                    resetAllPcConsumableState();
                    return;
                continue;
            }

            if (slot.isPlayer)
            {
                processPlayerTurn(slot.unit, state);
            }
            else
            {
                processEnemyTurn(slot.unit, state);
            }

            if ((checkAndHandleBattleEnd(state)))
            {
                resetAllPcConsumableState();
                return;
            }
        }
    }
}


void Battle::processActionResult(PlayableCharacter &actor,
                                 Party &allies,
                                 const ActionResult &result)
{
    if (result.spGained != 0)
        allies.gainSp(result.spGained);

    if (result.exposureDelta != 0)
        actor.modifyExposure(result.exposureDelta);
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
            {
                if (drop.type == Drop::Type::Gold ||
                    drop.type == Drop::Type::GuaranteedItem)
                {
                    m_playerParty.getInventory().gold += drop.goldAmount;
                }
            }
        }
    }
}