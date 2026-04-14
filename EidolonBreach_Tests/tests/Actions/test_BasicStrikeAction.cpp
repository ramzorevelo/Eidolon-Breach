/**
 * @file test_BasicStrikeAction.cpp
 * @brief Unit tests for BasicStrikeAction.
 */
#include "Actions/BasicStrikeAction.h"
#include "Entities/Enemy.h"
#include "Entities/IAIStrategy.h"
#include "Entities/Party.h"
#include "Entities/PlayableCharacter.h"
#include "doctest.h"
#include "test_helpers.h"
#include <memory>

TEST_CASE("BasicStrikeAction: deals damage and toughness, grants SP to party and Energy to user")
{
    Party allies, enemies;
    allies.gainSp(0); // start empty

    auto heroRaw = makeHero();
    auto *heroPtr = heroRaw.get();
    allies.addUnit(std::move(heroRaw));

    auto enemyRaw = makeEnemy(100, 50);
    auto *enemyPtr = enemyRaw.get();
    enemies.addUnit(std::move(enemyRaw));

    TargetInfo t{TargetInfo::Type::Enemy, 0};
    BasicStrikeAction action;
    ActionResult result = action.execute(*heroPtr, allies, enemies, t);

    CHECK(result.type == ActionResult::Type::Damage);
    CHECK(result.value == 15); // base power 15, 0 DEF
    CHECK(enemyPtr->getHp() == 85);
    CHECK(enemyPtr->getToughness() == 40); // 50 - 10 (kBasicToughDmg)
    CHECK(allies.getSp() == 15);           // +15 to shared pool
    CHECK(heroPtr->getEnergy() == 8);      // +8 Energy
}

TEST_CASE("BasicStrikeAction: DEF reduction formula applies")
{
    Party allies, enemies;
    allies.gainSp(0);

    auto heroRaw = makeHero();
    auto *heroPtr = heroRaw.get();
    allies.addUnit(std::move(heroRaw));

    auto enemyRaw = std::make_unique<Enemy>(
        "e", "HighDEF",
        Stats{100, 100, 10, 100, 5}, // DEF 100
        Affinity::Terra,
        50,
        std::make_unique<BasicAIStrategy>());
    auto *enemyPtr = enemyRaw.get();
    enemies.addUnit(std::move(enemyRaw));

    TargetInfo t{TargetInfo::Type::Enemy, 0};
    BasicStrikeAction action;
    ActionResult result = action.execute(*heroPtr, allies, enemies, t);

    // damage = max(1, 15 * (1 - 100/(100+100))) = max(1, 7.5) = 7
    CHECK(result.value == 7);
    CHECK(enemyPtr->getHp() == 93);
}

TEST_CASE("BasicStrikeAction: isAvailable always returns true")
{
    Party allies;
    auto hero = makeHero();
    BasicStrikeAction action;
    CHECK(action.isAvailable(*hero, allies) == true);
}

TEST_CASE("BasicStrikeAction: label contains correct resource info")
{
    BasicStrikeAction action;
    CHECK(action.label() == "Basic Strike (+15 SP | +8 Energy)");
}