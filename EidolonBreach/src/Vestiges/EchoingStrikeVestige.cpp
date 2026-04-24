/**
 * @file EchoingStrikeVestige.cpp
 * @brief EchoingStrikeVestige implementation.
 */

#include "Vestiges/EchoingStrikeVestige.h"
#include "Battle/Battle.h"
#include "Battle/BattleState.h"
#include "Core/ActionResult.h"
#include "Core/BattleEvents.h"
#include "Core/EventBus.h"
#include "Entities/Party.h"
#include "Entities/PlayableCharacter.h"

void EchoingStrikeVestige::onBattleStart(Battle & /*battle*/, BattleState &state)
{
    // Reset for this battle so a trigger from a previous battle doesn't carry over.
    m_nextActionFree = false;
    state.eventBus.subscribe<ResonanceFieldTriggeredEvent>(
        [this](const ResonanceFieldTriggeredEvent &)
        { m_nextActionFree = true; },
        EventScope::Battle);
}

void EchoingStrikeVestige::onAction(PlayableCharacter & /*actor*/,
                                    ActionResult &result,
                                    BattleState &state)
{
    if (!m_nextActionFree || result.spCost <= 0)
        return;

    // Refund the SP that was already spent inside execute().
    if (state.playerParty != nullptr)
        state.playerParty->gainSp(result.spCost);

    m_nextActionFree = false;
}

std::string EchoingStrikeVestige::getName() const
{
    return "Echoing Strike";
}

std::string EchoingStrikeVestige::getDescription() const
{
    return "After the Resonance Field triggers, your next SP-costing action is free.";
}