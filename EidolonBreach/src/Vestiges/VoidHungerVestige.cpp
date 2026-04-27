/**
 * @file VoidHungerVestige.cpp
 * @brief VoidHungerVestige implementation.
 */
#include "Vestiges/VoidHungerVestige.h"
#include "Battle/BattleState.h"
#include "Core/ActionResult.h"
#include "Entities/Party.h"
#include "Entities/PlayableCharacter.h"

void VoidHungerVestige::onTurnStart(PlayableCharacter &activeCharacter,
                                    BattleState & /*state*/)
{
    activeCharacter.modifyExposure(kExposurePerTurn);
}

void VoidHungerVestige::onAction(PlayableCharacter & /*actor*/,
                                 ActionResult &result,
                                 BattleState &state)
{
    if (result.type != ActionResult::Type::Damage || result.value <= 0)
        return;
    if (state.playerParty == nullptr)
        return;

    const int healAmt{result.value / kHealDivisor};
    if (healAmt <= 0)
        return;

    for (Unit *u : state.playerParty->getAliveUnits())
        u->heal(healAmt);
}

std::string VoidHungerVestige::getName() const
{
    return "Void Hunger";
}
std::string VoidHungerVestige::getDescription() const
{
    return "(Corrupted) Gain +8 Exposure each turn. "
           "Your damage actions heal all allies for 5% of damage dealt.";
}