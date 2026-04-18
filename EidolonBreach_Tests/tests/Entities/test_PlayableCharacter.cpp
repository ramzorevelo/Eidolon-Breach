/**
 * @file test_PlayableCharacter.cpp
 * @brief Unit tests for PlayableCharacter resource management.
 */
#include "Actions/BasicStrikeAction.h"
#include "Actions/SkillAction.h"
#include "Actions/UltimateAction.h"
#include "Entities/Party.h" 
#include "Entities/PlayableCharacter.h"
#include "doctest.h"
#include "test_helpers.h"
#include <memory>

TEST_CASE("PlayableCharacter: Energy management")
{
    auto hero = makeHero();
    CHECK(hero->getMomentum() == 0);
    CHECK(!hero->isUltimateReady());

    hero->gainMomentum(60);
    CHECK(hero->getMomentum() == 60);

    hero->gainMomentum(60);
    CHECK(hero->getMomentum() == 100);
    CHECK(hero->isUltimateReady());

    hero->resetMomentum();
    CHECK(hero->getMomentum() == 0);

    // Energy caps at kMaxEnergy (100)
    hero->gainMomentum(150);
    CHECK(hero->getMomentum() == 100);
}

TEST_CASE("PlayableCharacter: SP affordability delegates to Party")
{
    Party allies;
    allies.gainSp(30); // shared SP pool starts at 30

    auto hero = makeHero();
    // canAffordSp just checks party SP
    CHECK(hero->canAffordSp(20, allies) == true);
    CHECK(hero->canAffordSp(40, allies) == false);
}

TEST_CASE("PlayableCharacter: consumeSp reduces party SP")
{
    Party allies;
    allies.gainSp(50);

    auto hero = makeHero();
    hero->consumeSp(25, allies);
    CHECK(allies.getSp() == 25);
}

TEST_CASE("PlayableCharacter: resonanceContribution is owned by PlayableCharacter")
{
    auto hero = makeHero(); // test_helpers.h creates a hero with resonanceContribution = 10
    CHECK(hero->getResonanceContribution() == 10);
    CHECK(hero->getPassiveTrait() == "");
}

TEST_CASE("PlayableCharacter: Arch Skill threshold")
{
    auto hero = makeHero();
    CHECK(!hero->isArchSkillReady());

    hero->gainMomentum(39);
    CHECK(!hero->isArchSkillReady());

    hero->gainMomentum(1); // reaches 40
    CHECK(hero->isArchSkillReady());
    CHECK(!hero->isUltimateReady());

    hero->gainMomentum(60); // reaches 100
    CHECK(hero->isUltimateReady());
}