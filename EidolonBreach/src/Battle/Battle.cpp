#include "Battle/Battle.h"
#include "Battle/SpeedBasedTurnOrderCalculator.h"
#include "Core/Effects/BurnEffect.h"
#include "Core/Effects/ShieldEffect.h"
#include "Core/Effects/SlowEffect.h"
#include "Core/EffectIds.h"
#include "Entities/PlayableCharacter.h"
#include "Core/ActionResult.h"
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

void Battle::renderNewBreaks(const std::vector<bool> &before, const Party &party) const
{
    for (std::size_t i{0}; i < party.size() && i < before.size(); ++i)
    {
        const Unit *u{party.getUnitAt(i)};
        if (u && !before[i] && u->isBroken())
        {
            m_renderer.renderBreak(u->getName());
        }
    }
}

bool Battle::isBattleOver() const
{
    return m_playerParty.isAllDead() || m_enemyParty.isAllDead();
}

void Battle::processPlayerTurn(Unit *unit, BattleState &state)
{
    // FIXME Phase 6: replace dynamic_cast with Unit::onTurnStart() virtual hook
    // once BattleState is extended per TS §6.1
    // Tick arch skill cooldown at the start of each PC's turn.
    if (auto *pc = dynamic_cast<PlayableCharacter *>(unit))
        pc->tickArchSkillCooldown();

    auto breaksBefore{snapshotBreakStates(m_enemyParty)};
    ActionResult result{unit->takeTurn(m_playerParty, m_enemyParty, state)};
    m_renderer.renderActionResult(unit->getName(), result);
    renderNewBreaks(breaksBefore, m_enemyParty);

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

bool Battle::checkAndHandleBattleEnd()
{
    if (m_enemyParty.isAllDead())
    {
        for (std::size_t i{0}; i < m_enemyParty.size(); ++i)
        {
            Unit *u{m_enemyParty.getUnitAt(i)};
            if (!u->isAlive())
            {
                if (auto *e = dynamic_cast<Enemy *>(u))
                {
                    m_renderer.renderVictory(e->getName(), e->dropLoot());
                }
            }
        }
        return true;
    }

    if (m_playerParty.isAllDead())
    {
        for (std::size_t i{0}; i < m_playerParty.size(); ++i)
        {
            Unit *u{m_playerParty.getUnitAt(i)};
            if (!u->isAlive())
            {
                m_renderer.renderDefeat(u->getName());
            }
        }
        return true;
    }
    return false;
}

void Battle::run()
{
    m_renderer.renderMessage("\n=== BATTLE START ===");
    m_field.reset();
    BattleState state{0, 0, m_field, m_inputHandler, m_renderer};

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
                if (checkAndHandleBattleEnd())
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

            if (checkAndHandleBattleEnd())
            {
                return;
            }
        }
    }
}