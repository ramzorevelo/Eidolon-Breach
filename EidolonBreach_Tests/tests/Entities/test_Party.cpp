#include "doctest.h"
#include "Entities/Party.h"
#include "Entities/PlayableCharacter.h"
#include "Actions/BasicStrikeAction.h"
#include "Actions/SkillAction.h"
#include "Actions/UltimateAction.h"
#include <memory>
#include "test_helpers.h"


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

    const Unit* u = p.getUnitAt(1);
    CHECK(p.getIndex(u) == 1);
    CHECK(p.getIndex(nullptr) == p.size());
}