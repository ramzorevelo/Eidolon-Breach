/**
 * @file test_SurgingProc.cpp
 * @brief Tests for the Exposure threshold-75 "Surging" proc flag on PlayableCharacter.
 */
#include "Entities/PlayableCharacter.h"
#include "doctest.h"
#include "test_helpers.h"

TEST_CASE("PlayableCharacter: surgingProc starts disarmed")
{
    auto hero = makeHero();
    CHECK(!hero->isSurgingProcArmed());
}

TEST_CASE("PlayableCharacter: armSurgingProc sets the flag")
{
    auto hero = makeHero();
    hero->armSurgingProc();
    CHECK(hero->isSurgingProcArmed());
}

TEST_CASE("PlayableCharacter: consumeSurgingProc clears the flag")
{
    auto hero = makeHero();
    hero->armSurgingProc();
    hero->consumeSurgingProc();
    CHECK(!hero->isSurgingProcArmed());
}