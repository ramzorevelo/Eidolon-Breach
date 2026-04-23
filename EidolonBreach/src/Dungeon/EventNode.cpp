/**
 * @file EventNode.cpp
 * @brief EventNode stub — full event table added in a later phase.
 */

#include "Dungeon/EventNode.h"
#include "Entities/Party.h"
#include <iostream>

void EventNode::enter(Party &party,
                      MetaProgress & /*meta*/,
                      RunContext & /*runCtx*/,
                      EventBus & /*eventBus*/)
{
    std::cout << "\n=== MYSTERIOUS SIGNAL ===\n"
              << "A faint resonance echoes through the breach...\n"
              << "[Nothing of note occurs. Event table added in a later update.]\n";
    (void)party;
}

std::string EventNode::description() const
{
    return "[Event] An unknown encounter awaits.";
}