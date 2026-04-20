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
#include <ostream>

TEST_CASE("PlayableCharacter: Energy management")
{
    auto hero = makeHero();
    CHECK(hero->getEnergy() == 0);
    CHECK(!hero->isUltimateReady());

    hero->gainEnergy(60);
    CHECK(hero->getEnergy() == 60);

    hero->gainEnergy(60);
    CHECK(hero->getEnergy() == 100);
    CHECK(hero->isUltimateReady());

    hero->resetEnergy();
    CHECK(hero->getEnergy() == 0);

    // Energy caps at kMaxEnergy (100)
    hero->gainEnergy(150);
    CHECK(hero->getEnergy() == 100);
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

TEST_CASE("PlayableCharacter: Arch Skill threshold (placeholder — always ready)")
{
    auto hero = makeHero();
    // Placeholder: isArchSkillReady() returns true unconditionally.
    // This will be replaced with cooldown logic in future hotfix
    CHECK(hero->isArchSkillReady()); // Always ready regardless of energy

    hero->gainEnergy(39);
    CHECK(hero->isArchSkillReady());

    hero->gainEnergy(1);
    CHECK(hero->isArchSkillReady());
}