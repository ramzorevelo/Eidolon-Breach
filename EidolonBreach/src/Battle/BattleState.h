#pragma once
/**
 * @file BattleState.h
 * @brief Global battle context passed to takeTurn() and execute().
 *        Extend this struct rather than passing Battle& to game logic layers.
 */

#include "Core/Affinity.h"
#include "UI/IInputHandler.h"
#include "UI/IRenderer.h"

class ResonanceField;
class RunContext;
class EventBus;
class Party;

struct BattleState
{
    int currentFloor{0};
    int turnNumber{0};
    Affinity floorAffinity{Affinity::Aether};
    ResonanceField &resonanceField;
    IInputHandler &inputHandler;
    IRenderer &renderer;
    RunContext &runContext;
    EventBus &eventBus;
    Party *playerParty{nullptr};
    Party *enemyParty{nullptr};
};