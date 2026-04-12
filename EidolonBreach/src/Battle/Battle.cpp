#include "Battle/Battle.h"
#include "UI/ConsoleRenderer.h"
#include "Core/ActionResult.h"
#include <iostream>
#include <algorithm>
#include "Entities/Enemy.h"

Battle::Battle(Party& playerParty, Party& enemyParty)
    : m_playerParty{ playerParty }
    , m_enemyParty{ enemyParty }
{
}

// ── Turn order ────────────────────────────────────────────────────────────
std::vector<Battle::TurnSlot> Battle::buildTurnOrder() const
{
    std::vector<TurnSlot> slots;

    for (std::size_t i = 0; i < m_playerParty.size(); ++i)
    {
        Unit* u = m_playerParty.getUnitAt(i);
        if (u && u->isAlive()) slots.push_back({ u, true, i });
    }
    for (std::size_t i = 0; i < m_enemyParty.size(); ++i)
    {
        Unit* u = m_enemyParty.getUnitAt(i);
        if (u && u->isAlive()) slots.push_back({ u, false, i });
    }

    // Sort descending by SPD.
    // Tie-breaker 1: player party goes before enemies.
    // Tie-breaker 2: lower party index acts first.
    std::sort(slots.begin(), slots.end(), [](const TurnSlot& a, const TurnSlot& b)
        {
            int spdA = a.unit->getStats().spd;
            int spdB = b.unit->getStats().spd;
            if (spdA != spdB)              return spdA > spdB;
            if (a.isPlayer != b.isPlayer)  return a.isPlayer;   // player wins tie
            return a.partyIndex < b.partyIndex;
        });

    return slots;
}

// ── Break-state helpers ───────────────────────────────────────────────────
std::vector<bool> Battle::snapshotBreakStates(const Party& party) const
{
    std::vector<bool> states;
    states.reserve(party.size());
    for (std::size_t i = 0; i < party.size(); ++i)
    {
        const Unit* u = party.getUnitAt(i);
        states.push_back(u ? u->isBroken() : false);
    }
    return states;
}

void Battle::renderNewBreaks(const std::vector<bool>& before, const Party& party) const
{
    for (std::size_t i = 0; i < party.size() && i < before.size(); ++i)
    {
        const Unit* u = party.getUnitAt(i);
        if (u && !before[i] && u->isBroken())
            ConsoleRenderer::renderBreak(u->getName());
    }
}

// ── Main loop ─────────────────────────────────────────────────────────────
void Battle::run()
{
    std::cout << "\n=== BATTLE START ===\n";

    while (!m_playerParty.isAllDead() && !m_enemyParty.isAllDead())
    {
        ConsoleRenderer::printPartyStatus(m_playerParty, m_enemyParty);

        auto turnOrder = buildTurnOrder();

        for (const auto& slot : turnOrder)
        {
            if (!slot.unit->isAlive()) continue;
            if (m_playerParty.isAllDead() || m_enemyParty.isAllDead()) break;

            if (slot.isPlayer)
            {
                // Snapshot enemy break states before the player acts.
                auto breaksBefore = snapshotBreakStates(m_enemyParty);

                ActionResult result = slot.unit->takeTurn(m_playerParty, m_enemyParty);
                ConsoleRenderer::renderAttack(slot.unit->getName(), result);

                // Detect and announce freshly broken enemies.
                renderNewBreaks(breaksBefore, m_enemyParty);

                // Check victory immediately after player action.
                if (m_enemyParty.isAllDead())
                {
                    // Collect drops from every dead enemy.
                    for (std::size_t i = 0; i < m_enemyParty.size(); ++i)
                    {
                        Unit* u = m_enemyParty.getUnitAt(i);
                        if (!u->isAlive())
                        {
                            if (auto* e = dynamic_cast<Enemy*>(u))
                                ConsoleRenderer::renderVictory(e->getName(), e->dropLoot());
                        }
                    }
                    return;
                }
            }
            else
            {
                ActionResult result = slot.unit->takeTurn(m_enemyParty, m_playerParty);

                if (result.type == ActionResult::Type::Skip)
                    ConsoleRenderer::renderStunned(slot.unit->getName());
                else
                    ConsoleRenderer::renderAttack(slot.unit->getName(), result);

                // Check defeat immediately after enemy action.
                if (m_playerParty.isAllDead())
                {
                    for (std::size_t i = 0; i < m_playerParty.size(); ++i)
                    {
                        Unit* u = m_playerParty.getUnitAt(i);
                        if (!u->isAlive())
                            ConsoleRenderer::renderDefeat(u->getName());
                    }
                    return;
                }
            }
        }
    }
}