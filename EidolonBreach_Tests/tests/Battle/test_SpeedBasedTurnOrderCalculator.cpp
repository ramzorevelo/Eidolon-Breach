/**
 * @file test_SpeedBasedTurnOrderCalculator.cpp
 * @brief Unit tests for SpeedBasedTurnOrderCalculator.
 */
#include "Actions/BasicStrikeAction.h"
#include "Battle/SpeedBasedTurnOrderCalculator.h"
#include "Entities/Enemy.h"
#include "Entities/IAIStrategy.h"
#include "Entities/Party.h"
#include "Entities/PlayableCharacter.h"
#include "doctest.h"
#include <memory>

TEST_CASE("SpeedBasedTurnOrderCalculator: higher SPD acts first")
{
    Party playerParty, enemyParty;
    auto fastHero = std::make_unique<PlayableCharacter>(
        "fast", "Fast", Stats{100, 100, 10, 0, 20}, Affinity::Aether, 10);
    fastHero->addAbility(std::make_unique<BasicStrikeAction>());
    playerParty.addUnit(std::move(fastHero));

    auto slowEnemy = std::make_unique<Enemy>(
        "slow", "Slow", Stats{100, 100, 10, 0, 5}, Affinity::Blaze, 40,
        std::make_unique<BasicAIStrategy>());
    enemyParty.addUnit(std::move(slowEnemy));

    SpeedBasedTurnOrderCalculator calc;
    auto order = calc.calculate(playerParty, enemyParty);

    REQUIRE(order.size() == 2);
    CHECK(order[0].unit->getName() == "Fast");
    CHECK(order[1].unit->getName() == "Slow");
}

TEST_CASE("SpeedBasedTurnOrderCalculator: player before enemy on SPD tie")
{
    Party playerParty, enemyParty;
    auto hero = std::make_unique<PlayableCharacter>(
        "hero", "Hero", Stats{120, 120, 15, 0, 10}, Affinity::Aether, 10);
    hero->addAbility(std::make_unique<BasicStrikeAction>());
    playerParty.addUnit(std::move(hero));

    auto enemy = std::make_unique<Enemy>(
        "bat", "Bat", Stats{100, 100, 14, 0, 10}, Affinity::Blaze, 40,
        std::make_unique<BasicAIStrategy>());
    enemyParty.addUnit(std::move(enemy));

    SpeedBasedTurnOrderCalculator calc;
    auto order = calc.calculate(playerParty, enemyParty);

    REQUIRE(order.size() == 2);
    CHECK(order[0].isPlayer == true);
    CHECK(order[1].isPlayer == false);
}

TEST_CASE("SpeedBasedTurnOrderCalculator: lower index first when same SPD and same party")
{
    Party playerParty, enemyParty;
    auto hero1 = std::make_unique<PlayableCharacter>(
        "p1", "Hero1", Stats{100, 100, 10, 0, 10}, Affinity::Aether, 10);
    hero1->addAbility(std::make_unique<BasicStrikeAction>());
    playerParty.addUnit(std::move(hero1));

    auto hero2 = std::make_unique<PlayableCharacter>(
        "p2", "Hero2", Stats{100, 100, 10, 0, 10}, Affinity::Aether, 10);
    hero2->addAbility(std::make_unique<BasicStrikeAction>());
    playerParty.addUnit(std::move(hero2));

    SpeedBasedTurnOrderCalculator calc;
    auto order = calc.calculate(playerParty, enemyParty);

    REQUIRE(order.size() == 2);
    CHECK(order[0].partyIndex == 0);
    CHECK(order[1].partyIndex == 1);
}

TEST_CASE("SpeedBasedTurnOrderCalculator: dead units are excluded")
{
    Party playerParty, enemyParty;
    auto aliveHero = std::make_unique<PlayableCharacter>(
        "alive", "Alive", Stats{100, 100, 10, 0, 10}, Affinity::Aether, 10);
    aliveHero->addAbility(std::make_unique<BasicStrikeAction>());
    playerParty.addUnit(std::move(aliveHero));

    auto deadHero = std::make_unique<PlayableCharacter>(
        "dead", "Dead", Stats{100, 100, 10, 0, 20}, Affinity::Aether, 10);
    deadHero->addAbility(std::make_unique<BasicStrikeAction>());
    deadHero->takeDamage(9999);
    playerParty.addUnit(std::move(deadHero));

    SpeedBasedTurnOrderCalculator calc;
    auto order = calc.calculate(playerParty, enemyParty);

    REQUIRE(order.size() == 1);
    CHECK(order[0].unit->getName() == "Alive");
}

TEST_CASE("SpeedBasedTurnOrderCalculator: empty parties return empty vector")
{
    Party playerParty, enemyParty;
    SpeedBasedTurnOrderCalculator calc;
    auto order = calc.calculate(playerParty, enemyParty);
    CHECK(order.empty());
}