/**
 * @file TargetingHeuristics.cpp
 * @brief TargetingHeuristics implementation.
 */

#include "Battle/TargetingHeuristics.h"
#include "Entities/Party.h"
#include "Entities/Unit.h"
#include <limits>

namespace TargetingHeuristics
{

std::size_t defaultAllyTarget(TargetMode mode, const Party &allies)
{
    const auto alive{allies.getAliveUnits()};
    if (alive.empty())
        return 0;

    switch (mode)
    {
    case TargetMode::SingleAlly:
    case TargetMode::SplashAlly:
    {
        // Default: lowest current HP (heal heuristic).
        std::size_t bestIdx{0};
        int lowestHp{std::numeric_limits<int>::max()};
        for (std::size_t i{0}; i < alive.size(); ++i)
        {
            if (alive[i]->getHp() < lowestHp)
            {
                lowestHp = alive[i]->getHp();
                bestIdx = i;
            }
        }
        return bestIdx;
    }
    default:
        return 0;
    }
}

} // namespace TargetingHeuristics