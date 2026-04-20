/**
 * @file test_SkillAction.cpp
 * @brief Unit tests for SkillAction (Arch Skill [E]).
 */
#include "Actions/SkillAction.h"
#include "Entities/Enemy.h"
#include "Entities/IAIStrategy.h"
#include "Entities/Party.h"
#include "Entities/PlayableCharacter.h"
#include "doctest.h"
#include "test_helpers.h"
#include <memory>
#include <ostream>
#include <string>

TEST_CASE("SkillAction: requires 25 SP and 40 Momentum; consumes both on execute")
{
    Party allies, enemies;
    allies.gainSp(30); // enough SP for one use

    auto heroRaw = makeHero();
    auto *heroPtr = heroRaw.get();
    allies.addUnit(std::move(heroRaw));

    auto enemyRaw = makeEnemy(100, 50);
    auto *enemyPtr = enemyRaw.get();
    enemies.addUnit(std::move(enemyRaw));

    SkillAction skill{2.0f};

    // isAvailable requires both SP >= 25 AND Momentum >= 40.
    CHECK(!skill.isAvailable(*heroPtr, allies)); // momentum 0 — not ready

    heroPtr->gainEnergy(40);
    CHECK(skill.isAvailable(*heroPtr, allies)); // momentum 40, SP 30 — ready

    TargetInfo t{TargetInfo::Type::Enemy, 0};
    ActionResult result = skill.execute(*heroPtr, allies, enemies, t);

    // damage = 2.0 * 15 (ATK) * (1 - 0/100) = 30
    CHECK(result.value == 30);
    CHECK(enemyPtr->getHp() == 70);
    CHECK(enemyPtr->getToughness() == 25); // 50 - 25 (kSkillToughDmg)
    CHECK(allies.getSp() == 5);            // 30 - 25
    CHECK(heroPtr->getEnergy() == 0);    // 40 - 40 (momentum cost)
}

TEST_CASE("SkillAction: isAvailable returns false when SP insufficient even with enough Momentum")
{
    Party allies;
    allies.gainSp(20); // less than 25

    auto heroRaw = makeHero();
    auto *heroPtr = heroRaw.get();
    heroPtr->gainEnergy(40);
    allies.addUnit(std::move(heroRaw));

    SkillAction skill{};
    CHECK(!skill.isAvailable(*heroPtr, allies));
}

TEST_CASE("SkillAction: isAvailable returns false when Momentum insufficient even with enough SP")
{
    Party allies;
    allies.gainSp(50);

    auto heroRaw = makeHero();
    auto *heroPtr = heroRaw.get();
    // momentum stays at 0 — not ready
    allies.addUnit(std::move(heroRaw));

    SkillAction skill{};
    CHECK(!skill.isAvailable(*heroPtr, allies));
}

TEST_CASE("SkillAction: DEF reduction formula applies")
{
    Party allies, enemies;
    allies.gainSp(30);

    auto heroRaw = makeHero();
    auto *heroPtr = heroRaw.get();
    heroPtr->gainEnergy(40);
    allies.addUnit(std::move(heroRaw));

    auto enemyRaw = std::make_unique<Enemy>(
        "e", "HighDEF",
        Stats{100, 100, 10, 100, 5}, // DEF 100
        Affinity::Terra,
        50,
        std::make_unique<BasicAIStrategy>());
    auto *enemyPtr = enemyRaw.get();
    enemies.addUnit(std::move(enemyRaw));

    SkillAction skill{2.0f};
    TargetInfo t{TargetInfo::Type::Enemy, 0};
    ActionResult result = skill.execute(*heroPtr, allies, enemies, t);

    // damage = 2.0 * 15 * (1 - 100/200) = 30 * 0.5 = 15
    CHECK(result.value == 15);
    CHECK(enemyPtr->getHp() == 85);
}

TEST_CASE("SkillAction: execute with no SP still attempts (caller responsibility)")
{
    // execute() does not re-check resources — isAvailable guards normal flow.
    // useSp returns false when insufficient; SP stays 0.
    Party allies, enemies;
    allies.gainSp(0);

    auto heroRaw = makeHero();
    auto *heroPtr = heroRaw.get();
    allies.addUnit(std::move(heroRaw));
    enemies.addUnit(makeEnemy());

    SkillAction skill{};
    TargetInfo t{TargetInfo::Type::Enemy, 0};
    skill.execute(*heroPtr, allies, enemies, t);
    CHECK(allies.getSp() == 0);
}

TEST_CASE("SkillAction: label is correct")
{
    SkillAction skill{};
    CHECK(skill.label() == "Arch Skill (-25 SP | -40 Momentum)");
}