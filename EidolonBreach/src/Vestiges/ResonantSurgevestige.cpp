/**
 * @file ResonantSurgeVestige.cpp
 * @brief ResonantSurgeVestige implementation.
 */
#include "Vestiges/ResonantSurgeVestige.h"
#include "Battle/BattleState.h"
#include "Core/BattleEvents.h"
#include "Core/EventBus.h"
#include "Entities/Party.h"

void ResonantSurgeVestige::onBattleStart(Battle & /*battle*/, BattleState &state)
{
    state.eventBus.subscribe<ResonanceFieldTriggeredEvent>(
        [this, &state](const ResonanceFieldTriggeredEvent &)
        {
            if (state.playerParty != nullptr)
                state.playerParty->gainSp(kSpBonus);
        },
        EventScope::Battle);
}

std::string ResonantSurgeVestige::getName() const
{
    return "Resonant Surge";
}
std::string ResonantSurgeVestige::getDescription() const
{
    return "After the Resonance Field triggers, the party gains +20 SP.";
}