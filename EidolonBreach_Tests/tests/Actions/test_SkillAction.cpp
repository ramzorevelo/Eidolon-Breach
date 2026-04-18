/**
 * @file test_SkillAction.cpp
 * @brief Unit tests for SkillAction.
 */
#include "Actions/SkillAction.h"
#include "Entities/Enemy.h"
#include "Entities/Party.h"
#include "Entities/PlayableCharacter.h"
#include "doctest.h"
#include "test_helpers.h"
#include <memory>

TEST_CASE("SkillAction: requires and consumes 25 SP from party")
{
    Party allies, enemies;
    allies.gainSp(30); // enough for one use

    auto heroRaw = makeHero();
    auto *heroPtr = heroRaw.get();
    allies.addUnit(std::move(heroRaw));

    auto enemyRaw = makeEnemy(100, 50);
    auto *enemyPtr = enemyRaw.get();
    enemies.addUnit(std::move(enemyRaw));

    SkillAction skill(28); // custom damage
    CHECK(skill.isAvailable(*heroPtr, allies));

    TargetInfo t{TargetInfo::Type::Enemy, 0};
    ActionResult result = skill.execute(*heroPtr, allies, enemies, t);

    CHECK(result.value == 28); // 0 DEF -> full damage
    CHECK(enemyPtr->getHp() == 72);
    CHECK(enemyPtr->getToughness() == 25); // 50 - 25 (kSkillToughDmg)
    CHECK(allies.getSp() == 5);            // 30 - 25
    CHECK(heroPtr->getMomentum() == 15);     // +15 Energy
}

TEST_CASE("SkillAction: isAvailable returns false when party SP insufficient")
{
    Party allies, enemies;
    allies.gainSp(20); // less than 25

    auto hero = makeHero();
    allies.addUnit(std::move(hero));

    SkillAction skill;
    CHECK(!skill.isAvailable(*dynamic_cast<PlayableCharacter *>(allies.getUnitAt(0)), allies));
}

TEST_CASE("SkillAction: cannot execute if SP insufficient (caller responsibility)")
{
    Party allies, enemies;
    allies.gainSp(0);

    auto heroRaw = makeHero();
    auto *heroPtr = heroRaw.get();
    allies.addUnit(std::move(heroRaw));
    enemies.addUnit(makeEnemy());

    SkillAction skill;
    // execute() does not re-check SP; assumes caller already checked.
    // In normal flow, isAvailable prevents this call.
    // We test that it still consumes SP even if it goes negative (should not happen).
    // Since useSp() returns false when insufficient, this is safe.
    TargetInfo t{TargetInfo::Type::Enemy, 0};
    // This will attempt to use 25 SP when only 0 available -> useSp returns false, SP stays 0.
    skill.execute(*heroPtr, allies, enemies, t);
    CHECK(allies.getSp() == 0);
}

TEST_CASE("SkillAction: label displays SP cost and Energy gain")
{
    SkillAction skill;
    CHECK(skill.label() == "Skill (-25 SP | +15 Energy)");
}