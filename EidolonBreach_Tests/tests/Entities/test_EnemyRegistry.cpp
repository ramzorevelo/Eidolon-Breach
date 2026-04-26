/**
 * @file test_EnemyRegistry.cpp
 * @brief Tests for EnemyRegistry loading and enemy instantiation.
 */
#include "Entities/Enemy.h"
#include "Entities/EnemyRegistry.h"
#include "doctest.h"

TEST_CASE("EnemyRegistry: size is non-zero after loadFromJson")
{
    EnemyRegistry reg{};
    reg.loadFromJson("data/enemies.json");
    CHECK(reg.size() > 0);
}

TEST_CASE("EnemyRegistry: create returns nullptr for unknown id")
{
    EnemyRegistry reg{};
    reg.loadFromJson("data/enemies.json");
    CHECK(reg.create("nonexistent") == nullptr);
}

TEST_CASE("EnemyRegistry: create returns valid Enemy for registered id")
{
    EnemyRegistry reg{};
    reg.loadFromJson("data/enemies.json");
    auto enemy{reg.create("slime_alpha")};
    REQUIRE(enemy != nullptr);
    CHECK(enemy->getName() == "Slime Alpha");
    CHECK(enemy->getMaxHp() == 80);
    CHECK(enemy->getMaxToughness() == 30);
    CHECK(enemy->isAlive());
}

TEST_CASE("EnemyRegistry: boss variant has correct stats")
{
    EnemyRegistry reg{};
    reg.loadFromJson("data/enemies.json");
    auto warden{reg.create("breach_warden")};
    REQUIRE(warden != nullptr);
    CHECK(warden->getName() == "Breach Warden");
    CHECK(warden->getMaxHp() == 200);
    CHECK(warden->getMaxToughness() == 90);
}

TEST_CASE("EnemyRegistry: contains returns true for registered id")
{
    EnemyRegistry reg{};
    reg.loadFromJson("data/enemies.json");
    CHECK(reg.contains("vampire_bat"));
    CHECK(!reg.contains("dragon_king"));
}