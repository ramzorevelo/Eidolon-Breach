#include "Battle/Battle.h"
#include "Battle/SpeedBasedTurnOrderCalculator.h"
#include "Core/ActionResult.h"
#include "Entities/Enemy.h"
#include "UI/ConsoleRenderer.h"
#include <algorithm>
#include <iostream>

Battle::Battle(Party &playerParty,
               Party &enemyParty,
               std::unique_ptr<ITurnOrderCalculator> turnOrderCalc)
    : m_playerParty{playerParty}, m_enemyParty{enemyParty}, m_turnOrderCalc{turnOrderCalc ? std::move(turnOrderCalc)
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
            ConsoleRenderer::renderBreak(u->getName());
        }
    }
}

bool Battle::isBattleOver() const
{
    return m_playerParty.isAllDead() || m_enemyParty.isAllDead();
}

void Battle::processPlayerTurn(Unit *unit)
{
    auto breaksBefore{snapshotBreakStates(m_enemyParty)};
    ActionResult result{unit->takeTurn(m_playerParty, m_enemyParty)};
    ConsoleRenderer::renderAttack(unit->getName(), result);
    renderNewBreaks(breaksBefore, m_enemyParty);
}

void Battle::processEnemyTurn(Unit *unit)
{
    ActionResult result{unit->takeTurn(m_enemyParty, m_playerParty)};
    if (result.type == ActionResult::Type::Skip)
    {
        ConsoleRenderer::renderStunned(unit->getName());
    }
    else
    {
        ConsoleRenderer::renderAttack(unit->getName(), result);
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
                    ConsoleRenderer::renderVictory(e->getName(), e->dropLoot());
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
                ConsoleRenderer::renderDefeat(u->getName());
            }
        }
        return true;
    }
    return false;
}

void Battle::run()
{
    std::cout << "\n=== BATTLE START ===\n";

    while (!isBattleOver())
    {
        ConsoleRenderer::printPartyStatus(m_playerParty, m_enemyParty);

        auto turnOrder{m_turnOrderCalc->calculate(m_playerParty, m_enemyParty)};

        for (const auto &slot : turnOrder)
        {
            if (!slot.unit->isAlive())
                continue;
            if (isBattleOver())
                break;

            if (slot.isPlayer)
            {
                processPlayerTurn(slot.unit);
            }
            else
            {
                processEnemyTurn(slot.unit);
            }

            if (checkAndHandleBattleEnd())
            {
                return;
            }
        }
    }
}