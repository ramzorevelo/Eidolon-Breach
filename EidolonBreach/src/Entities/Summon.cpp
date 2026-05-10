/**
 * @file Summon.cpp
 * @brief Summon implementation.
 */

#include "Entities/Summon.h"
#include "Battle/BattleState.h"
#include "Entities/Party.h"
#include <algorithm>

Summon::Summon(const SummonDefinition &def, int summonerContribution, int summonerAtk)
    : Unit{def.id, def.displayName, def.baseStats, Affinity::Aether},
      m_definition{&def},
      m_resonanceContribution{summonerContribution / 2},
      m_remainingTurns{def.duration},
      m_summonerAtk{summonerAtk}
{
}

ActionResult Summon::takeTurn(Party &allies, Party &enemies, BattleState &state)
{
    if (!m_definition || m_definition->actions.empty())
        return ActionResult{ActionResult::Type::Skip, 0};

    const SummonAction &action{
        m_definition->actions[m_actionIndex % m_definition->actions.size()]};
    ++m_actionIndex;

    return action.execute(*this, allies, enemies, state);
}

int Summon::getResonanceContribution() const
{
    return m_resonanceContribution;
}

bool Summon::isExpired() const
{
    return m_remainingTurns.has_value() && *m_remainingTurns <= 0;
}

void Summon::tickDuration()
{
    if (m_remainingTurns.has_value())
        m_remainingTurns = std::max(0, *m_remainingTurns - 1);
}

int Summon::getSummonerAtk() const
{
    return m_summonerAtk;
}

bool Summon::tickSummonLifecycle()
{
    tickDuration();
    return isExpired();
}