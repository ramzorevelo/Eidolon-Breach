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
#include <ostream>

TEST_CASE("BasicStrikeAction: sets spGained = 15 in result; does not touch party SP directly")
{
    Party allies, enemies;
    allies.gainSp(0);

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
    CHECK(result.value == 15);
    CHECK(result.spGained == 15); // Battle applies this to the party pool
    CHECK(allies.getSp() == 0);   // action no longer calls gainSp() directly
    CHECK(enemyPtr->getHp() == 85);
    CHECK(enemyPtr->getToughness() == 40); // 50 - 10 (kBasicToughDmg)
    CHECK(heroPtr->getEnergy() == 25);     // +25 Energy to user
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

TEST_CASE("BasicStrikeAction: exposureDelta is 0 (no exposure change)")
{
    Party allies, enemies;
    auto heroRaw = makeHero();
    auto *heroPtr = heroRaw.get();
    allies.addUnit(std::move(heroRaw));
    enemies.addUnit(makeEnemy());

    BasicStrikeAction action;
    TargetInfo t{TargetInfo::Type::Enemy, 0};
    ActionResult result = action.execute(*heroPtr, allies, enemies, t);
    CHECK(result.exposureDelta == 0);
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
    CHECK(action.label() == "Basic Strike (+15 SP | +25 Energy)");
}