#include "Entities/IAIStrategy.h"
#include "Party.h"

/** 
 * @file IAIStrategy.h
 * @brief AI decision interface for enemies.
 */
AIDecision BasicAIStrategy::decide(const Unit& /*self*/, const Party& targets)
{
    for (std::size_t i{ 0 }; i < targets.size(); ++i)
    {
        const Unit* u{ targets.getUnitAt(i) };
        if (u && u->isAlive())
            return AIDecision{ i };
    }
    return AIDecision{ 0 };
}