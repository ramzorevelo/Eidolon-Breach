/**
 * @file SwiftStrikeVestige.cpp
 * @brief SwiftStrikeVestige implementation.
 */
#include "Vestiges/SwiftStrikeVestige.h"
#include "Battle/BattleState.h"
#include "Entities/PlayableCharacter.h"

void SwiftStrikeVestige::onTurnStart(PlayableCharacter &activeCharacter,
                                     BattleState & /*state*/)
{
    if (activeCharacter.getHp() == activeCharacter.getMaxHp())
        activeCharacter.gainEnergy(kEnergyBonus);
}

std::string SwiftStrikeVestige::getName() const
{
    return "Swift Strike";
}
std::string SwiftStrikeVestige::getDescription() const
{
    return "At the start of your turn, if your HP is full, gain +5 Energy.";
}