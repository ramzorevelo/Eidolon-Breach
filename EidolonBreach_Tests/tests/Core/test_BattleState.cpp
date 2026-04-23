/**
 * @file test_BattleState.cpp
 * @brief Tests for BattleState construction.
 */
#include "Battle/BattleState.h"
#include "Battle/ResonanceField.h"
#include "doctest.h"
#include <ostream>
#include <UI/test_NullRenderer.h>
#include <UI/test_NullInputHandler.h>
#include "Core/EventBus.h"
#include "Core/RunContext.h"

TEST_CASE("BattleState: holds references to all injected dependencies")
{
    ResonanceField field{};
    NullInputHandler inputHandler{};
    NullRenderer renderer{};
    RunContext runContext{};
    EventBus eventBus{};
    BattleState state{0, 0, Affinity::Aether, field, inputHandler, renderer, runContext, eventBus};

    CHECK(state.currentFloor == 0);
    CHECK(state.turnNumber == 0);
    CHECK(state.floorAffinity == Affinity::Aether);
    CHECK(&state.resonanceField == &field);
    CHECK(&state.inputHandler == &inputHandler);
    CHECK(&state.renderer == &renderer);
}


TEST_CASE("BattleState: turnNumber is independently modifiable")
{
    ResonanceField field{};
    NullInputHandler inputHandler{};
    NullRenderer renderer{};
    RunContext runContext{};
    EventBus eventBus{};
    BattleState state{0, 0, Affinity::Aether, field, inputHandler, renderer, runContext, eventBus};
    ++state.turnNumber;
    CHECK(state.turnNumber == 1);
}