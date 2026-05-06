/**
 * @file EventNode.cpp
 * @brief EventNode stub — full event table added in a later phase.
 */

#include "Dungeon/EventNode.h"
#include "Entities/Party.h"
#include "UI/IRenderer.h"
#include "UI/IInputHandler.h"

void EventNode::enter(Party &party, MetaProgress & /*meta*/,
                      RunContext & /*runCtx*/, EventBus & /*eventBus*/,
                      IRenderer &renderer, IInputHandler & /*input*/)
{
    renderer.renderMessage("=== MYSTERIOUS SIGNAL ===");
    renderer.renderMessage("A faint resonance echoes through the breach...");
    renderer.renderMessage("[Nothing of note occurs. Event table added later.]");
    renderer.presentPause(600);
    (void)party;
}

std::string EventNode::description() const
{
    return "[Event] An unknown encounter awaits.";
}