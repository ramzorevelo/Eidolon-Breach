#include "doctest.h"
#include "Entities/Enemy.h"
#include "Entities/Party.h"
#include <memory>
#include "test_helpers.h"


TEST_CASE("Enemy: toughness break and recovery")
{
    auto e = makeEnemy(100, 50);
    CHECK(!e->isBroken());
    CHECK(e->getToughness() == 50);
    CHECK(e->getMaxToughness() == 50);

    e->applyToughnessHit(30);
    CHECK(e->getToughness() == 20);
    CHECK(!e->isBroken());

    e->applyToughnessHit(30);
    CHECK(e->isBroken());
    CHECK(e->getToughness() == 50);

    e->recoverFromBreak();
    CHECK(!e->isBroken());
}

TEST_CASE("Enemy: toughness does not go below zero")
{
    auto e = makeEnemy(100, 50);
    e->applyToughnessHit(40);
    CHECK(e->getToughness() == 10);
    e->applyToughnessHit(5);
    CHECK(e->getToughness() == 5);
}

TEST_CASE("Enemy: takeTurn returns Skip when broken")
{
    auto e = makeEnemy(100, 10);
    e->applyToughnessHit(10);
    REQUIRE(e->isBroken());

    Party playerParty, enemyParty;

    ActionResult result = e->takeTurn(enemyParty, playerParty);
    CHECK(result.type == ActionResult::Type::Skip);
    CHECK(!e->isBroken());
}