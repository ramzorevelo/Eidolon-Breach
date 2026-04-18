/**
 * @file test_BattleState.cpp
 * @brief Tests for BattleState construction.
 */
#include "Battle/BattleState.h"
#include "Battle/ResonanceField.h"
#include "doctest.h"
#include <ostream>

TEST_CASE("BattleState: holds reference to ResonanceField")
{
    ResonanceField field{};
    BattleState state{0, 0, field};
    CHECK(state.currentFloor == 0);
    CHECK(state.turnNumber == 0);
    CHECK(&state.resonanceField == &field);
}

TEST_CASE("BattleState: turnNumber is independently modifiable")
{
    ResonanceField field{};
    BattleState state{0, 0, field};
    ++state.turnNumber;
    CHECK(state.turnNumber == 1);
}