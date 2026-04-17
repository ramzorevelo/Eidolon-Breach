/**
 * @file test_ShieldEffect.cpp
 * @brief Tests for ShieldEffect absorption and exhaustion.
 */
#include "Core/EffectIds.h"
#include "Core/Effects/ShieldEffect.h"
#include "doctest.h"
#include "test_helpers.h"
#include <ostream>
TEST_CASE("ShieldEffect: absorbs damage fully when pool is sufficient")
{
    ShieldEffect shield{50, 3};
    CHECK(shield.absorbDamage(30) == 0);
    CHECK(!shield.isExhausted());
}

TEST_CASE("ShieldEffect: returns overflow when incoming exceeds pool")
{
    ShieldEffect shield{30, 3};
    int overflow = shield.absorbDamage(50);
    CHECK(overflow == 20);
    CHECK(shield.isExhausted());
}

TEST_CASE("ShieldEffect: exact hit depletes pool and exhausts shield")
{
    ShieldEffect shield{40, 3};
    int overflow = shield.absorbDamage(40);
    CHECK(overflow == 0);
    CHECK(shield.isExhausted());
}

TEST_CASE("ShieldEffect: applied to unit — partial overflow reaches HP")
{
    auto hero = makeHero(); // 120 HP
    hero->applyEffect(std::make_unique<ShieldEffect>(20, 3));
    hero->takeDamage(50);
    // Shield absorbs 20; 30 overflows → 120 - 30 = 90
    CHECK(hero->getHp() == 90);
}

TEST_CASE("ShieldEffect: correct ID, tags, and categorisation")
{
    ShieldEffect shield{50, 3};
    CHECK(shield.getId() == EffectIds::kShield);
    CHECK(shield.isBuff() == true);
    CHECK(shield.isDebuff() == false);
    CHECK(shield.hasTag(EffectTags::kShield));
    CHECK(shield.hasTag(EffectTags::kTerra));
    CHECK(shield.hasTag(EffectTags::kBuff));
}

TEST_CASE("ShieldEffect: does not modify stats")
{
    ShieldEffect shield{50, 3};
    Stats base{100, 100, 10, 5, 10};
    CHECK(shield.modifyStatsFlat(base).atk == base.atk);
    CHECK(shield.modifyStatsPct(base).spd == base.spd);
}