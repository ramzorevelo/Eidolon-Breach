#pragma once
/**
 * @file BattleState.h
 * @brief Global battle context passed to takeTurn()
 */

#include "UI/IInputHandler.h"
#include "UI/IRenderer.h"

class ResonanceField;

struct BattleState
{
    int currentFloor{0};
    int turnNumber{0};
    ResonanceField &resonanceField;
    IInputHandler &inputHandler;
    IRenderer &renderer;
};