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

TEST_CASE("PlayableCharacter: arch skill cooldown starts at 0 (ready)")
{
    auto hero = makeHero();
    CHECK(hero->isArchSkillReady());
    CHECK(hero->getArchSkillCooldown() == 0);
}

TEST_CASE("PlayableCharacter: consumeArchSkill sets cooldown to kArchSkillCooldownTurns")
{
    auto hero = makeHero();
    hero->consumeArchSkill();
    CHECK(!hero->isArchSkillReady());
    CHECK(hero->getArchSkillCooldown() == 2);
}

TEST_CASE("PlayableCharacter: tickArchSkillCooldown decrements and clamps at 0")
{
    auto hero = makeHero();
    hero->consumeArchSkill(); // cooldown = 2
    hero->tickArchSkillCooldown();
    CHECK(hero->getArchSkillCooldown() == 1);
    hero->tickArchSkillCooldown();
    CHECK(hero->getArchSkillCooldown() == 0);
    CHECK(hero->isArchSkillReady());

    // Extra tick must not go negative.
    hero->tickArchSkillCooldown();
    CHECK(hero->getArchSkillCooldown() == 0);
}
TEST_CASE("PlayableCharacter: slots start locked")
{
    auto hero = makeHero();
    const auto &equipped = hero->getEquippedSkills();
    CHECK(!equipped.slots[0].unlocked);
    CHECK(!equipped.slots[1].unlocked);
    CHECK(!equipped.slots[0].isReady());
    CHECK(!equipped.slots[1].isReady());
}

TEST_CASE("PlayableCharacter: tryUnlockSlot unlocks the correct slot")
{
    auto hero = makeHero();
    hero->tryUnlockSlot(0);
    CHECK(hero->getEquippedSkills().slots[0].unlocked);
    CHECK(!hero->getEquippedSkills().slots[1].unlocked);

    hero->tryUnlockSlot(1);
    CHECK(hero->getEquippedSkills().slots[1].unlocked);
}

TEST_CASE("PlayableCharacter: tryUnlockSlot out-of-range is a no-op")
{
    auto hero = makeHero();
    hero->tryUnlockSlot(-1); // must not crash
    hero->tryUnlockSlot(2);  // must not crash
    CHECK(!hero->getEquippedSkills().slots[0].unlocked);
    CHECK(!hero->getEquippedSkills().slots[1].unlocked);
}

TEST_CASE("PlayableCharacter: tryUnlockSlot twice is a no-op (idempotent)")
{
    auto hero = makeHero();
    hero->tryUnlockSlot(0);
    hero->tryUnlockSlot(0); // second call must not crash
    CHECK(hero->getEquippedSkills().slots[0].unlocked);
}