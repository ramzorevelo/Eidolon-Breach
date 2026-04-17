/**
 * @file test_Unit.cpp
 * @brief Unit tests for Unit base class HP management.
 */
#include "Actions/BasicStrikeAction.h"
#include "Actions/SkillAction.h"
#include "Actions/UltimateAction.h"
#include "Entities/PlayableCharacter.h"
#include "Entities/Unit.h"
#include "doctest.h"
#include "test_helpers.h"
#include <memory>

TEST_CASE("Unit: HP management")
{
    auto hero = makeHero();
    CHECK(hero->getHp() == 120);
    CHECK(hero->getMaxHp() == 120);
    CHECK(hero->isAlive());

    hero->takeDamage(50);
    CHECK(hero->getHp() == 70);

    hero->takeDamage(999);
    CHECK(hero->getHp() == 0);
    CHECK(!hero->isAlive());

    hero->heal(30);
    CHECK(hero->getHp() == 30);

    hero->heal(999);
    CHECK(hero->getHp() == 120);
}

TEST_CASE("Unit: isBroken returns false by default (PlayableCharacter)")
{
    auto hero = makeHero();
    CHECK(!hero->isBroken());
    hero->applyToughnessHit(9999);
    CHECK(!hero->isBroken());
}

TEST_CASE("Unit: getBaseStats returns construction-time stats unchanged")
{
    auto hero = makeHero();
    const Stats &base = hero->getBaseStats();
    // makeHero uses Stats{120, 120, 15, 0, 10}
    CHECK(base.hp == 120);
    CHECK(base.maxHp == 120);
    CHECK(base.atk == 15);
    CHECK(base.def == 0);
    CHECK(base.spd == 10);
}

TEST_CASE("Unit: getFinalStats equals getBaseStats when no effects are active")
{
    auto hero = makeHero();
    Stats base = hero->getBaseStats();
    Stats final = hero->getFinalStats();
    CHECK(final.hp == base.hp);
    CHECK(final.maxHp == base.maxHp);
    CHECK(final.atk == base.atk);
    CHECK(final.def == base.def);
    CHECK(final.spd == base.spd);
}
