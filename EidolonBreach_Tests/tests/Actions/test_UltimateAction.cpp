/**
 * @file test_UltimateAction.cpp
 * @brief Unit tests for UltimateAction.
 */
#include "Actions/UltimateAction.h"
#include "Entities/Enemy.h"
#include "Entities/Party.h"
#include "Entities/PlayableCharacter.h"
#include "doctest.h"
#include "test_helpers.h"
#include <memory>
#include <ostream>
#include <string>

TEST_CASE("UltimateAction: requires full Momentum, resets it and refunds 5")
{
    Party allies, enemies;
    allies.gainSp(50);

    auto heroRaw = makeHero();
    auto *heroPtr = heroRaw.get();
    allies.addUnit(std::move(heroRaw));

    auto enemyRaw = makeEnemy(100, 50);
    auto *enemyPtr = enemyRaw.get();
    enemies.addUnit(std::move(enemyRaw));

    UltimateAction ult;
    CHECK(!ult.isAvailable(*heroPtr, allies)); // momentum 0

    heroPtr->gainEnergy(100);
    CHECK(ult.isAvailable(*heroPtr, allies)); // momentum 100

    TargetInfo t{TargetInfo::Type::Enemy, 0};
    ActionResult result = ult.execute(*heroPtr, allies, enemies, t);

    // damage = 4.0 * 15 (ATK) * (1 - 0/100) = 60
    CHECK(result.value == 60);
    CHECK(enemyPtr->getHp() == 40);
    CHECK(enemyPtr->getToughness() == 20); // 50 - 30 (kUltToughDmg)
    CHECK(heroPtr->getEnergy() == 5);    // reset to 0 then +5 refund
    CHECK(allies.getSp() == 50);           // ultimate does not affect SP
}

TEST_CASE("UltimateAction: does not consume SP")
{
    Party allies, enemies;
    allies.gainSp(50);

    auto heroRaw = makeHero();
    auto *heroPtr = heroRaw.get();
    allies.addUnit(std::move(heroRaw));
    enemies.addUnit(makeEnemy());

    heroPtr->gainEnergy(100);
    UltimateAction ult;
    ult.execute(*heroPtr, allies, enemies, TargetInfo{TargetInfo::Type::Enemy, 0});
    CHECK(allies.getSp() == 50);
}

TEST_CASE("UltimateAction: label is correct")
{
    UltimateAction ult;
    CHECK(ult.label() == "Ultimate (full Momentum -> 0 | +5 Momentum)");
}