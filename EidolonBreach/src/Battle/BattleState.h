#pragma once
/**
 * @file BattleState.h
 * @brief Global battle context passed to takeTurn()
 */

#include "UI/IInputHandler.h"
#include "UI/IRenderer.h"

class ResonanceField;
class Party;

struct BattleState
{
    int currentFloor{0};
    int turnNumber{0};
    ResonanceField &resonanceField;
    IInputHandler &inputHandler;
    IRenderer &renderer;
    /// Populated by Battle::run() — nullptr until then.
    Party *playerParty{nullptr};
    /// Populated by Battle::run() — nullptr until then.
    Party *enemyParty{nullptr};
};