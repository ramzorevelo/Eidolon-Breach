#pragma once
/**
 * @file BattleState.h
 * @brief Global battle context passed to takeTurn()
 */

class ResonanceField;

struct BattleState
{
    int currentFloor{0};
    int turnNumber{0};
    ResonanceField &resonanceField;
};