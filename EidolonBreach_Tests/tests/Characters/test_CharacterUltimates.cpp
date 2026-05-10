/**
 * @file test_CharacterUltimates.cpp
 * @brief Tests for LyraUltimateAction, VexUltimateAction, ZaraUltimateAction.
 */
#include "Characters/Lyra/LyraUltimateAction.h"
#include "Characters/Vex/VexUltimateAction.h"
#include "Characters/Zara/ZaraUltimateAction.h"
#include "Core/EffectIds.h"
#include "Entities/Party.h"
#include "doctest.h"
#include "test_helpers.h"
#include <memory>


TEST_CASE("LyraUltimateAction: requires full Energy")
{
    Party allies;
    auto hero = makeHero();
    LyraUltimateAction ult{};
    CHECK(!ult.isAvailable(*hero, allies));
    hero->gainEnergy(100);
    CHECK(ult.isAvailable(*hero, allies));
}

TEST_CASE("LyraUltimateAction: deals damage and applies Burn")
{
    Party allies, enemies;
    auto heroRaw = makeHero();
    auto *hero = heroRaw.get();
    hero->gainEnergy(100);
    allies.addUnit(std::move(heroRaw));
    auto enemyRaw = makeEnemy(100, 50);
    auto *enemy = enemyRaw.get();
    enemies.addUnit(std::move(enemyRaw));

    LyraUltimateAction ult{};
    TargetInfo t{TargetInfo::Type::Enemy, 0};
    ActionResult result = ult.execute(*hero, allies, enemies, t);

    CHECK(result.type == ActionResult::Type::Damage);
    CHECK(result.value > 0);
    CHECK(enemy->getHp() < 100);
    CHECK(enemy->hasEffect(EffectIds::kBurn));
    CHECK(hero->getEnergy() == 5);
}

TEST_CASE("LyraUltimateAction: affinity is Blaze")
{
    LyraUltimateAction ult{};
    CHECK(ult.getAffinity() == Affinity::Blaze);
}


TEST_CASE("VexUltimateAction: shields all allies")
{
    Party allies, enemies;
    auto h1 = makeHero("h1");
    auto h2 = makeHero("h2");
    auto *p1 = h1.get();
    auto *p2 = h2.get();
    p1->gainEnergy(100);
    allies.addUnit(std::move(h1));
    allies.addUnit(std::move(h2));

    VexUltimateAction ult{};
    ult.execute(*p1, allies, enemies, std::nullopt);

    CHECK(p1->hasEffect(EffectIds::kShield));
    CHECK(p2->hasEffect(EffectIds::kShield));
    CHECK(p1->getEnergy() == 5);
}

TEST_CASE("VexUltimateAction: grants SP to party")
{
    Party allies, enemies;
    allies.gainSp(10);
    auto heroRaw = makeHero();
    auto *hero = heroRaw.get();
    hero->gainEnergy(100);
    allies.addUnit(std::move(heroRaw));

    VexUltimateAction ult{};
    ult.execute(*hero, allies, enemies, std::nullopt);

    CHECK(allies.getSp() == 40); // 10 + 30
}

TEST_CASE("VexUltimateAction: affinity is Terra")
{
    VexUltimateAction ult{};
    CHECK(ult.getAffinity() == Affinity::Terra);
}


TEST_CASE("ZaraUltimateAction: slows all enemies and deals damage")
{
    Party allies, enemies;
    auto heroRaw = makeHero();
    auto *hero = heroRaw.get();
    hero->gainEnergy(100);
    allies.addUnit(std::move(heroRaw));
    auto e1 = makeEnemy(100, 50);
    auto e2 = makeEnemy(100, 50);
    auto *ep1 = e1.get();
    auto *ep2 = e2.get();
    enemies.addUnit(std::move(e1));
    enemies.addUnit(std::move(e2));

    ZaraUltimateAction ult{};
    ActionResult result = ult.execute(*hero, allies, enemies, std::nullopt);

    CHECK(result.type == ActionResult::Type::Damage);
    CHECK(ep1->getHp() < 100);
    CHECK(ep2->getHp() < 100);
    CHECK(ep1->hasEffect(EffectIds::kSlow));
    CHECK(ep2->hasEffect(EffectIds::kSlow));
    CHECK(hero->getEnergy() == 5);
}

TEST_CASE("ZaraUltimateAction: affinity is Frost")
{
    ZaraUltimateAction ult{};
    CHECK(ult.getAffinity() == Affinity::Frost);
}