/**
 * @file AttunistGambitVestige.cpp
 * @brief AttunistGambitVestige implementation.
 */

#include "Vestiges/AttunistGambitVestige.h"
#include "Battle/Battle.h"
#include "Battle/BattleState.h"
#include "Core/ActionResult.h"
#include "Core/BattleEvents.h"
#include "Core/EventBus.h"
#include "Entities/Party.h"
#include "Entities/PlayableCharacter.h"
#include <algorithm>

void AttunistGambitVestige::onBattleStart(Battle & /*battle*/, BattleState &state)
{
    m_exposureReducedThisBattle = 0;
    state.eventBus.subscribe<UnitDefeatedEvent>(
        [this, &state](const UnitDefeatedEvent &e)
        {
            if (!e.killer || m_exposureReducedThisBattle >= kMaxReductionPerBattle)
                return;
            // Only player-side kills trigger the reduction. Use the player party to
            // verify the killer is a Synchron without requiring a downcast.
            if (state.playerParty == nullptr ||
                !state.playerParty->contains(e.killer))
                return;
            // dynamic_cast is justified here: the vestige interface specifically
            // operates on PlayableCharacter, and playerParty is confirmed to contain
            // the unit. This is the one place in Vestiges/ where the cast is
            // intentional and documented.
            auto *pc{dynamic_cast<PlayableCharacter *>(e.killer)};
            if (!pc)
                return;
            const int reduction{
                std::min(kExposureOnKill,
                         kMaxReductionPerBattle - m_exposureReducedThisBattle)};
            pc->modifyExposure(-reduction);
            m_exposureReducedThisBattle += reduction;
        },
        EventScope::Battle);
}

void AttunistGambitVestige::onAction(PlayableCharacter & /*actor*/,
                                     ActionResult &result,
                                     BattleState & /*state*/)
{
    // Amplify positive Exposure gains only (negative = reduction, not amplified).
    if (result.exposureDelta > 0)
    {
        result.exposureDelta = static_cast<int>(
            static_cast<float>(result.exposureDelta) * kExposureGainMultiplier);
    }
}

void AttunistGambitVestige::onBattleEnd(BattleState & /*state*/)
{
    m_exposureReducedThisBattle = 0;
}

std::string AttunistGambitVestige::getName() const
{
    return "Attunist's Gambit";
}

std::string AttunistGambitVestige::getDescription() const
{
    return "(Corrupted) Kills reduce killer Exposure by 8 (max 40/battle). "
           "All Exposure gains are increased by 50%.";
}