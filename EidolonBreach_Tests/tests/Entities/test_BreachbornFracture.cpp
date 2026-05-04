/**
 * @file test_BreachbornFracture.cpp
 * @brief Tests for Breachborn and Fracture state transitions on PlayableCharacter.
 */
#include "Entities/PlayableCharacter.h"
#include "doctest.h"
#include "test_helpers.h"

TEST_CASE("PlayableCharacter: starts not Breachborn and not Fractured")
{
    auto hero = makeHero();
    CHECK(!hero->isBreachbornActive());
    CHECK(!hero->isFractured());
}

TEST_CASE("PlayableCharacter: activateBreachborn sets active for 3 turns")
{
    auto hero = makeHero();
    hero->activateBreachborn();
    CHECK(hero->isBreachbornActive());
    CHECK(!hero->isFractured());
    CHECK(hero->getBreachbornTurnsRemaining() == 3);
}

TEST_CASE("PlayableCharacter: tickBreachborn decrements counter")
{
    auto hero = makeHero();
    hero->activateBreachborn();
    bool ended{false};

    ended = hero->tickBreachborn();
    CHECK(!ended);
    CHECK(hero->getBreachbornTurnsRemaining() == 2);
    CHECK(hero->isBreachbornActive());

    ended = hero->tickBreachborn();
    CHECK(!ended);
    CHECK(hero->getBreachbornTurnsRemaining() == 1);

    ended = hero->tickBreachborn(); // 3rd tick — Breachborn ends
    CHECK(ended);
    CHECK(!hero->isBreachbornActive());
    CHECK(hero->isFractured());
}

TEST_CASE("PlayableCharacter: activateBreachborn while Fractured refreshes counter")
{
    auto hero = makeHero();
    hero->activateBreachborn();
    hero->tickBreachborn();
    hero->tickBreachborn();
    hero->tickBreachborn(); // now Fractured
    REQUIRE(hero->isFractured());

    hero->activateBreachborn(); // re-hit 100 Exposure while Fractured
    CHECK(hero->isBreachbornActive());
    CHECK(hero->isFractured()); // Fracture persists
    CHECK(hero->getBreachbornTurnsRemaining() == 3);
}

TEST_CASE("PlayableCharacter: tickBreachborn on inactive character returns false")
{
    auto hero = makeHero();
    CHECK(!hero->tickBreachborn());
    CHECK(!hero->isFractured());
}