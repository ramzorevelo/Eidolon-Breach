/**
 * @file test_Party.cpp
 * @brief Unit tests for Party container and shared resources.
 */
#include "Actions/BasicStrikeAction.h"
#include "Actions/SkillAction.h"
#include "Actions/UltimateAction.h"
#include "Entities/Party.h"
#include "Entities/PlayableCharacter.h"
#include "doctest.h"
#include "test_helpers.h"
#include <memory>

TEST_CASE("Party: addUnit, size, isAllDead")
{
    Party p;
    CHECK(p.size() == 0);
    CHECK(p.isAllDead());

    p.addUnit(makeHero("h1"));
    p.addUnit(makeHero("h2"));
    CHECK(p.size() == 2);
    CHECK(!p.isAllDead());

    p.getUnitAt(0)->takeDamage(9999);
    CHECK(!p.isAllDead());

    p.getUnitAt(1)->takeDamage(9999);
    CHECK(p.isAllDead());
}

TEST_CASE("Party: getAliveUnits excludes dead units")
{
    Party p;
    p.addUnit(makeHero("h1"));
    p.addUnit(makeHero("h2"));
    p.addUnit(makeHero("h3"));

    p.getUnitAt(1)->takeDamage(9999);

    auto alive = p.getAliveUnits();
    CHECK(alive.size() == 2);
    CHECK(alive[0]->getId() == "h1");
    CHECK(alive[1]->getId() == "h3");
}

TEST_CASE("Party: getIndex finds correct position")
{
    Party p;
    p.addUnit(makeHero("h1"));
    p.addUnit(makeHero("h2"));

    const Unit *u = p.getUnitAt(1);
    CHECK(p.getIndex(u) == 1);
    CHECK(p.getIndex(nullptr) == p.size());
}

TEST_CASE("Party: shared SP management")
{
    Party p;
    CHECK(p.getSp() == 0);
    CHECK(p.getMaxSp() == 100); // kDefaultMaxSp

    p.gainSp(30);
    CHECK(p.getSp() == 30);
    p.gainSp(80);
    CHECK(p.getSp() == 100); // capped

    CHECK(p.useSp(40) == true);
    CHECK(p.getSp() == 60);
    CHECK(p.useSp(100) == false);
    CHECK(p.getSp() == 60); // unchanged on failure
}

TEST_CASE("Party: SP defaults to 0, must be initialised externally")
{
    Party p;
    CHECK(p.getSp() == 0);
    // The main game loop is responsible for setting starting SP (50 per spec)
    p.gainSp(50);
    CHECK(p.getSp() == 50);
}