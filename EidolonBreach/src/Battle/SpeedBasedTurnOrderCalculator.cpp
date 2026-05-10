#include "Battle/SpeedBasedTurnOrderCalculator.h"
#include "Entities/Party.h"
#include "Entities/Unit.h"
#include <algorithm>

std::vector<TurnSlot> SpeedBasedTurnOrderCalculator::calculate(
    Party &playerParty,
    Party &enemyParty) const
{

    std::vector<TurnSlot> slots{};

    // Collect alive player units
    for (std::size_t i{0}; i < playerParty.size(); ++i)
    {
        Unit *u{playerParty.getUnitAt(i)};
        if (u && u->isAlive())
        {
            slots.push_back({u, true, i});
        }
    }

    // Collect alive enemy units
    for (std::size_t i{0}; i < enemyParty.size(); ++i)
    {
        Unit *u{enemyParty.getUnitAt(i)};
        if (u && u->isAlive())
        {
            slots.push_back({u, false, i});
        }
    }

    // Sort descending by SPD.
    // Tie‑breaker 1: player party goes before enemies.
    // Tie‑breaker 2: lower party index acts first.
    std::sort(slots.begin(), slots.end(),
              [](const TurnSlot &a, const TurnSlot &b)
              {
                  int spdA{a.unit->getFinalStats().spd};
                  int spdB{b.unit->getFinalStats().spd};
                  if (spdA != spdB)
                  {
                      return spdA > spdB;
                  }
                  if (a.isPlayer != b.isPlayer)
                  {
                      return a.isPlayer; // player wins tie
                  }
                  return a.partyIndex < b.partyIndex;
              });

    return slots;
}