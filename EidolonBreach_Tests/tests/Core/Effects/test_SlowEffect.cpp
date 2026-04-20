/**
 * @file test_SlowEffect.cpp
 * @brief Tests for SlowEffect SPD reduction via modifyStatsPct.
 */
#include "Core/EffectIds.h"
#include "Core/Effects/SlowEffect.h"
#include "Core/Stats.h"
#include "doctest.h"
#include "test_helpers.h"

TEST_CASE("SlowEffect: modifyStatsPct reduces SPD by slowRatio percentage")
{
    SlowEffect slow{0.30f, 2};
    Stats base{100, 100, 10, 5, 10};
    // 10 * 0.30 = 3 removed → SPD 7
    Stats result = slow.modifyStatsPct(base);
    CHECK(result.spd == 7);
    CHECK(result.atk == base.atk); // other stats unchanged
    CHECK(result.def == base.def);
}

TEST_CASE("SlowEffect: modifyStatsFlat is a no-op (SPD reduction is in pct pass)")
{
    SlowEffect slow{0.30f, 2};
    Stats base{100, 100, 10, 5, 10};
    Stats result = slow.modifyStatsFlat(base);
    CHECK(result.spd == base.spd); // flat pass must not change SPD
}

TEST_CASE("SlowEffect: applied to unit — getFinalStats reflects percentage reduction")
{
    // makeHero SPD = 10; 10 * 0.30 = 3 removed → SPD 7
    auto hero = makeHero();
    hero->applyEffect(std::make_unique<SlowEffect>(0.30f, 2));
    CHECK(hero->getFinalStats().spd == 7);
    CHECK(hero->getBaseStats().spd == 10); // base unchanged
}

TEST_CASE("SlowEffect: correct ID, tags, and categorisation")
{
    SlowEffect slow{};
    CHECK(slow.getId() == EffectIds::kSlow);
    CHECK(slow.isDebuff() == true);
    CHECK(slow.isBuff() == false);
    CHECK(slow.hasTag(EffectTags::kStatMod));
    CHECK(slow.hasTag(EffectTags::kFrost));
    CHECK(slow.hasTag(EffectTags::kDebuff));
}

TEST_CASE("SlowEffect: 50% reduction applied via percentage pass")
{
    auto hero = makeHero();                                    // SPD 10
    hero->applyEffect(std::make_unique<SlowEffect>(0.50f, 2)); // -50% → SPD 5
    CHECK(hero->getFinalStats().spd == 5);
}