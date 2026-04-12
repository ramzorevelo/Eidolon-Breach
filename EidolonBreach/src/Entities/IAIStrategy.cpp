#include "Entities/IAIStrategy.h"
#include "Party.h"   // included here to keep the header lightweight

AIDecision BasicAIStrategy::decide(const Unit& /*self*/, const Party& targets)
{
    // Pick the first alive unit.
    for (std::size_t i = 0; i < targets.size(); ++i)
    {
        const Unit* u = targets.getUnitAt(i);
        if (u && u->isAlive())
            return AIDecision{ i };
    }
    return AIDecision{ 0 };
}