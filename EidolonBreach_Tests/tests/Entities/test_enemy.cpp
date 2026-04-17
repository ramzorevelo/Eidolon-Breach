/**
 * @file test_Enemy.cpp
 * @brief Unit tests for Enemy toughness and AI.
 */
#include "Entities/Enemy.h"
#include "Entities/Party.h"
#include "doctest.h"
#include "test_helpers.h"
#include <memory>

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
    CHECK(e->getToughness() == 50); // resets to max

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
    CHECK(!e->isBroken()); // recovers after skip
}

TEST_CASE("Enemy::takeTurn applies DEF reduction to damage")
{
    // Player unit with nonzero DEF (Conduit archetype baseline DEF 15)
    auto player = std::make_unique<PlayableCharacter>(
        "test", "TestChar",
        Stats{100, 100, 20, 15, 10}, // DEF = 15
        Affinity::Frost, 10);

    Party playerParty;
    playerParty.addUnit(std::move(player));

    // Subclass Enemy to override performAttack() with fixed 30 damage
    class FixedDamageEnemy : public Enemy
    {
      public:
        using Enemy::Enemy;

      protected:
        ActionResult performAttack() override
        {
            return {ActionResult::Type::Damage, 30};
        }
    };

    auto enemy = std::make_unique<FixedDamageEnemy>(
        "fix", "FixedDam",
        Stats{50, 50, 10, 5, 5},
        Affinity::Blaze,
        20,                                 // maxToughness
        std::make_unique<BasicAIStrategy>() // AI strategy (targeting handled separately)
    );

    Party enemyParty;
    Unit *enemyPtr = enemy.get();
    enemyParty.addUnit(std::move(enemy));

    // Force enemy turn (targets first alive player unit via AI)
    enemyPtr->takeTurn(enemyParty, playerParty);

    Unit *target = playerParty.getUnitAt(0);
    // DEF 15 with K=100 gives mitigation = 15/(115) ≈ 0.13 → damage ≈ 30 * 0.87 ≈ 26
    // Expected HP: 100 - 26 = 74
    CHECK(target->getHp() == 74);
}