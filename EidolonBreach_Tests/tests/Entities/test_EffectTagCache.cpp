/**
 * @file test_EffectTagCache.cpp
 * @brief Tests for the EffectTagFlag bitmask cache on Unit.
 */
#include "Core/EffectIds.h"
#include "Core/Effects/BurnEffect.h"
#include "Core/Effects/ShieldEffect.h"
#include "Core/Effects/SlowEffect.h"
#include "doctest.h"
#include "test_helpers.h"

TEST_CASE("Unit: hasEffectWithTag false when no effects applied")
{
    auto hero = makeHero();
    CHECK(!hero->hasEffectWithTag(EffectTags::kDebuff));
    CHECK(!hero->hasEffectWithTag(EffectTags::kBuff));
}

TEST_CASE("Unit: hasEffectWithTag true after applying tagged effect")
{
    auto hero = makeHero();
    hero->applyEffect(std::make_unique<BurnEffect>(5, 2));
    CHECK(hero->hasEffectWithTag(EffectTags::kDebuff));
    CHECK(hero->hasEffectWithTag(EffectTags::kDoT));
    CHECK(!hero->hasEffectWithTag(EffectTags::kBuff));
}

TEST_CASE("Unit: hasEffectWithTag false after removeEffectsByTag")
{
    auto hero = makeHero();
    hero->applyEffect(std::make_unique<BurnEffect>(5, 2));
    REQUIRE(hero->hasEffectWithTag(EffectTags::kDebuff));

    hero->removeEffectsByTag(EffectTags::kDebuff);
    CHECK(!hero->hasEffectWithTag(EffectTags::kDebuff));
    CHECK(!hero->hasEffectWithTag(EffectTags::kDoT));
}

TEST_CASE("Unit: removeEffectsByTag early-exit preserves unrelated effects")
{
    auto hero = makeHero();
    hero->applyEffect(std::make_unique<ShieldEffect>(20, 2));
    hero->applyEffect(std::make_unique<BurnEffect>(5, 2));

    // Removing debuffs should not touch the shield buff.
    hero->removeEffectsByTag(EffectTags::kDebuff);
    CHECK(!hero->hasEffectWithTag(EffectTags::kDebuff));
    CHECK(hero->hasEffectWithTag(EffectTags::kBuff));
    CHECK(hero->hasEffectWithTag(EffectTags::kShield));
}

TEST_CASE("Unit: tag cache cleared after effect expires via tickEffects")
{
    auto hero = makeHero();
    hero->applyEffect(std::make_unique<SlowEffect>(0.20f, 1)); // expires after 1 tick
    REQUIRE(hero->hasEffectWithTag(EffectTags::kDebuff));

    (void)hero->tickEffects(); // expires the slow
    CHECK(!hero->hasEffectWithTag(EffectTags::kDebuff));
}

TEST_CASE("Unit: multiple effects — cache reflects all active tags")
{
    auto hero = makeHero();
    hero->applyEffect(std::make_unique<BurnEffect>(5, 2));
    hero->applyEffect(std::make_unique<ShieldEffect>(20, 2));

    CHECK(hero->hasEffectWithTag(EffectTags::kDebuff));
    CHECK(hero->hasEffectWithTag(EffectTags::kBuff));
    CHECK(hero->hasEffectWithTag(EffectTags::kShield));
    CHECK(hero->hasEffectWithTag(EffectTags::kDoT));
}