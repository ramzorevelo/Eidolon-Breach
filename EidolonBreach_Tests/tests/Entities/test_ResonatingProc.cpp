/**
 * @file test_ResonatingProc.cpp
 * @brief Tests for the Exposure threshold-50 "Resonating" proc flag on PlayableCharacter.
 */
#include "Entities/PlayableCharacter.h"
#include "doctest.h"
#include "test_helpers.h"

TEST_CASE("PlayableCharacter: resonatingProc starts disarmed")
{
    auto hero = makeHero();
    CHECK(!hero->isResonatingProcArmed());
}

TEST_CASE("PlayableCharacter: armResonatingProc sets the flag")
{
    auto hero = makeHero();
    hero->armResonatingProc();
    CHECK(hero->isResonatingProcArmed());
}

TEST_CASE("PlayableCharacter: consumeResonatingProc clears the flag")
{
    auto hero = makeHero();
    hero->armResonatingProc();
    hero->consumeResonatingProc();
    CHECK(!hero->isResonatingProcArmed());
}