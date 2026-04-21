/**
 * @file test_TargetingHeuristics.cpp
 * @brief Tests for TargetingHeuristics::defaultAllyTarget().
 */
#include "Battle/TargetingHeuristics.h"
#include "Entities/Party.h"
#include "doctest.h"
#include "test_helpers.h"

TEST_CASE("TargetingHeuristics: SingleAlly selects lowest-HP ally")
{
    Party allies;
    auto h1 = makeHero("h1"); // 120 HP
    auto h2 = makeHero("h2"); // 120 HP
    auto h3 = makeHero("h3"); // 120 HP
    h2->takeTrueDamage(80);   // h2 at 40 HP
    allies.addUnit(std::move(h1));
    allies.addUnit(std::move(h2));
    allies.addUnit(std::move(h3));

    std::size_t idx = TargetingHeuristics::defaultAllyTarget(
        TargetMode::SingleAlly, allies);
    // Alive units in order: h1 (120), h2 (40), h3 (120).
    // getAliveUnits() returns them in insertion order.
    // Index 1 is h2 with lowest HP.
    CHECK(idx == 1);
}

TEST_CASE("TargetingHeuristics: returns 0 when all allies have equal HP")
{
    Party allies;
    allies.addUnit(makeHero("h1"));
    allies.addUnit(makeHero("h2"));

    std::size_t idx = TargetingHeuristics::defaultAllyTarget(
        TargetMode::SingleAlly, allies);
    CHECK(idx == 0);
}

TEST_CASE("TargetingHeuristics: returns 0 for non-ally modes")
{
    Party allies;
    allies.addUnit(makeHero("h1"));

    CHECK(TargetingHeuristics::defaultAllyTarget(TargetMode::SingleEnemy, allies) == 0);
    CHECK(TargetingHeuristics::defaultAllyTarget(TargetMode::Self, allies) == 0);
}

TEST_CASE("TargetingHeuristics: returns 0 for empty party (no crash)")
{
    Party empty;
    CHECK(TargetingHeuristics::defaultAllyTarget(TargetMode::SingleAlly, empty) == 0);
}