/**
 * @file test_SkillAction.cpp
 * @brief Unit tests for SkillAction (Arch Skill [E]) — cooldown-gated.
 */
#include "Actions/SkillAction.h"
#include "Entities/Enemy.h"
#include "Entities/IAIStrategy.h"
#include "Entities/Party.h"
#include "Entities/PlayableCharacter.h"
#include "doctest.h"
#include "test_helpers.h"
#include <memory>

TEST_CASE("SkillAction: available when arch skill cooldown is 0")
{
    Party allies;
    auto heroRaw = makeHero();
    auto *heroPtr = heroRaw.get();
    allies.addUnit(std::move(heroRaw));
    heroPtr->applyUnlocks(20);
    SkillAction skill{2.0f};
    CHECK(skill.isAvailable(*heroPtr, allies)); // cooldown starts at 0
}

TEST_CASE("SkillAction: unavailable immediately after consumeArchSkill()")
{
    Party allies;
    auto heroRaw = makeHero();
    auto *heroPtr = heroRaw.get();
    allies.addUnit(std::move(heroRaw));
    heroPtr->applyUnlocks(20);
    SkillAction skill{2.0f};
    heroPtr->consumeArchSkill();
    CHECK(!skill.isAvailable(*heroPtr, allies));
}

TEST_CASE("SkillAction: available again after cooldown expires (2 ticks)")
{
    Party allies;
    auto heroRaw = makeHero();
    auto *heroPtr = heroRaw.get();
    allies.addUnit(std::move(heroRaw));
    heroPtr->applyUnlocks(20);
    SkillAction skill{2.0f};
    heroPtr->consumeArchSkill();
    CHECK(!skill.isAvailable(*heroPtr, allies));

    heroPtr->tickArchSkillCooldown();
    CHECK(!skill.isAvailable(*heroPtr, allies)); // still on cooldown

    heroPtr->tickArchSkillCooldown();
    CHECK(skill.isAvailable(*heroPtr, allies)); // cooldown expired
}

TEST_CASE("SkillAction: execute calls consumeArchSkill (cooldown set after use)")
{
    Party allies, enemies;
    allies.gainSp(50);

    auto heroRaw = makeHero();
    auto *heroPtr = heroRaw.get();
    allies.addUnit(std::move(heroRaw));
    enemies.addUnit(makeEnemy(100, 50));
    heroPtr->applyUnlocks(20);
    SkillAction skill{2.0f};
    REQUIRE(skill.isAvailable(*heroPtr, allies));
    TargetInfo t{TargetInfo::Type::Enemy, 0};
    skill.execute(*heroPtr, allies, enemies, t);

    // After execute, cooldown should be set.
    CHECK(!skill.isAvailable(*heroPtr, allies));
    CHECK(heroPtr->getArchSkillCooldown() == 2);
}

TEST_CASE("SkillAction: deals damage scaled by skillPower")
{
    Party allies, enemies;

    auto heroRaw = makeHero();
    auto *heroPtr = heroRaw.get();
    allies.addUnit(std::move(heroRaw));

    auto enemyRaw = makeEnemy(100, 50);
    auto *enemyPtr = enemyRaw.get();
    enemies.addUnit(std::move(enemyRaw));
    heroPtr->applyUnlocks(20);
    SkillAction skill{2.0f};
    TargetInfo t{TargetInfo::Type::Enemy, 0};
    ActionResult result = skill.execute(*heroPtr, allies, enemies, t);

    // damage = 2.0 * 15 (ATK) * (1 - 0/100) = 30
    CHECK(result.value == 30);
    CHECK(enemyPtr->getHp() == 70);
    CHECK(enemyPtr->getToughness() == 25); // 50 - 25 (kSkillToughDmg)
}

TEST_CASE("SkillAction: label is correct")
{
    SkillAction skill{};
    CHECK(skill.label() == "Arch Skill (2-turn cooldown)");
}

TEST_CASE("SkillAction: unavailable when arch skill not yet unlocked (level 1)")
{
    Party allies;
    auto hero = makeHero();
    SkillAction skill{2.0f};
    CHECK(!skill.isAvailable(*hero, allies));
}

TEST_CASE("SkillAction: available after applyUnlocks(20) and cooldown is 0")
{
    Party allies;
    auto hero = makeHero();
    hero->applyUnlocks(20);
    SkillAction skill{2.0f};
    CHECK(skill.isAvailable(*hero, allies));
}