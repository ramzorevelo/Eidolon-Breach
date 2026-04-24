/**
 * @file VestigeOfTheUnbound.cpp
 * @brief VestigeOfTheUnbound implementation.
 */

#include "Vestiges/VestigeOfTheUnbound.h"
#include "Battle/BattleState.h"
#include "Entities/PlayableCharacter.h"

void VestigeOfTheUnbound::onTurnStart(PlayableCharacter &activeCharacter,
                                      BattleState & /*state*/)
{
    activeCharacter.modifyExposure(kExposurePerTurn);
}

std::string VestigeOfTheUnbound::getName() const
{
    return "Vestige of the Unbound";
}

std::string VestigeOfTheUnbound::getDescription() const
{
    return "(Corrupted) Gain +5 Exposure at the start of each of your turns.";
}