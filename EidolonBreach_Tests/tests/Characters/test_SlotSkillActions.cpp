/**
 * @file test_SlotSkillActions.cpp
 * @brief Tests for VexBulwarkAction and ZaraFrostbindAction.
 */
#include "Characters/Vex/VexBulwarkAction.h"
#include "Characters/Zara/ZaraFrostbindAction.h"
#include "Core/EffectIds.h"
#include "Entities/Party.h"
#include "doctest.h"
#include "test_helpers.h"

TEST_CASE("VexBulwarkAction: unavailable when party cannot afford SP")
{
    Party allies;
    allies.gainSp(10);
    auto hero = makeHero();
    VexBulwarkAction action{};
    CHECK(!action.isAvailable(*hero, allies));
}

TEST_CASE("VexBulwarkAction: applies ShieldEffect to target ally and costs SP")
{
    Party allies, enemies;
    allies.gainSp(50);
    auto heroRaw = makeHero();
    auto *hero = heroRaw.get();
    allies.addUnit(std::move(heroRaw));

    VexBulwarkAction action{};
    TargetInfo t{TargetInfo::Type::Ally, 0};
    action.execute(*hero, allies, enemies, t);

    CHECK(hero->hasEffect(EffectIds::kShield));
    CHECK(allies.getSp() == 25);
}

TEST_CASE("ZaraFrostbindAction: applies SlowEffect and costs SP")
{
    Party allies, enemies;
    allies.gainSp(50);
    auto heroRaw = makeHero();
    auto *hero = heroRaw.get();
    allies.addUnit(std::move(heroRaw));
    auto enemyRaw = makeEnemy(100, 50);
    auto *enemy = enemyRaw.get();
    enemies.addUnit(std::move(enemyRaw));

    ZaraFrostbindAction action{};
    TargetInfo t{TargetInfo::Type::Enemy, 0};
    action.execute(*hero, allies, enemies, t);

    CHECK(enemy->hasEffect(EffectIds::kSlow));
    CHECK(allies.getSp() == 25);
}

TEST_CASE("ZaraFrostbindAction: unavailable when party cannot afford SP")
{
    Party allies;
    allies.gainSp(10);
    auto hero = makeHero();
    ZaraFrostbindAction action{};
    CHECK(!action.isAvailable(*hero, allies));
}