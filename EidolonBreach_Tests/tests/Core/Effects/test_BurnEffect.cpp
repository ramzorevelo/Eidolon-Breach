/**
 * @file test_BurnEffect.cpp
 * @brief Tests for BurnEffect DoT behavior.
 */
#include "Core/EffectIds.h"
#include "Core/Effects/BurnEffect.h"
#include "Core/Effects/ShieldEffect.h"
#include "doctest.h"
#include "test_helpers.h"
#include <ostream>

TEST_CASE("BurnEffect: deals true damage on tick — hero HP is reduced")
{
    auto hero = makeHero(); // 120 HP, DEF 0
    BurnEffect burn{10, 2};
    std::string msg = burn.onTick(*hero);
    CHECK(hero->getHp() == 110);
    CHECK(!msg.empty());
}

TEST_CASE("BurnEffect: bypasses shield absorption (takeTrueDamage path)")
{
    auto hero = makeHero(); // 120 HP
    hero->applyEffect(std::make_unique<ShieldEffect>(100, 3));
    BurnEffect burn{20, 2};
    burn.onTick(*hero);
    // takeTrueDamage bypasses shield: HP must drop.
    CHECK(hero->getHp() == 100);
}

TEST_CASE("BurnEffect: correct ID, tags, and categorisation")
{
    BurnEffect burn{5, 2};
    CHECK(burn.getId() == EffectIds::kBurn);
    CHECK(burn.isDebuff() == true);
    CHECK(burn.isBuff() == false);
    CHECK(burn.hasTag(EffectTags::kDoT));
    CHECK(burn.hasTag(EffectTags::kBlaze));
    CHECK(burn.hasTag(EffectTags::kDebuff));
}

TEST_CASE("BurnEffect: does not modify stats (flat or pct)")
{
    BurnEffect burn{5, 2};
    Stats base{100, 100, 15, 5, 10};
    CHECK(burn.modifyStatsFlat(base).spd == base.spd);
    CHECK(burn.modifyStatsPct(base).atk == base.atk);
}

TEST_CASE("BurnEffect: absorbs no direct damage (not a shield)")
{
    BurnEffect burn{5, 2};
    CHECK(burn.absorbDamage(30) == 30);
}