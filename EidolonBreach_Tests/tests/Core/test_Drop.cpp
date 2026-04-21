/**
 * @file test_Drop.cpp
 * @brief Tests for Drop struct and Enemy::generateDrops().
 */
#include "Core/Drop.h"
#include "Entities/Enemy.h"
#include "Entities/IAIStrategy.h"
#include "doctest.h"
#include "test_helpers.h"

TEST_CASE("Enemy::generateDrops: GuaranteedItem always appears")
{
    auto enemy = makeEnemy(100, 50);
    enemy->addDrop(Drop{Drop::Type::GuaranteedItem, 0, "item_a", 1.0f});

    auto drops = enemy->generateDrops(0u);
    REQUIRE(drops.size() == 1);
    CHECK(drops[0].itemId == "item_a");
}

TEST_CASE("Enemy::generateDrops: Gold drop with chance 1.0 always appears")
{
    auto enemy = makeEnemy(100, 50);
    enemy->addDrop(Drop{Drop::Type::Gold, 20, {}, 1.0f});

    auto drops = enemy->generateDrops(0u);
    REQUIRE(drops.size() == 1);
    CHECK(drops[0].type == Drop::Type::Gold);
    CHECK(drops[0].goldAmount == 20);
}

TEST_CASE("Enemy::generateDrops: Drop with chance 0.0 never appears")
{
    auto enemy = makeEnemy(100, 50);
    enemy->addDrop(Drop{Drop::Type::Gold, 20, {}, 0.0f});

    auto drops = enemy->generateDrops(0u);
    CHECK(drops.empty());
}

TEST_CASE("Enemy::generateDrops: empty pool returns empty vector")
{
    auto enemy = makeEnemy(100, 50);
    auto drops = enemy->generateDrops(0u);
    CHECK(drops.empty());
}

TEST_CASE("Enemy::generateDrops: multiple drops rolled independently")
{
    auto enemy = makeEnemy(100, 50);
    enemy->addDrop(Drop{Drop::Type::Gold, 20, {}, 1.0f});
    enemy->addDrop(Drop{Drop::Type::GuaranteedItem, 0, "key_item", 1.0f});

    auto drops = enemy->generateDrops(0u);
    CHECK(drops.size() == 2);
}

TEST_CASE("Enemy::generateDrops: same seed produces same result (deterministic)")
{
    auto e1 = makeEnemy(100, 50);
    auto e2 = makeEnemy(100, 50);
    e1->addDrop(Drop{Drop::Type::Gold, 10, {}, 0.5f});
    e2->addDrop(Drop{Drop::Type::Gold, 10, {}, 0.5f});

    auto drops1 = e1->generateDrops(42u);
    auto drops2 = e2->generateDrops(42u);
    CHECK(drops1.size() == drops2.size());
}