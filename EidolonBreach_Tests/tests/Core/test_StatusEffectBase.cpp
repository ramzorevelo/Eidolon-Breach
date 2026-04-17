/**
 * @file test_StatusEffectBase.cpp
 * @brief Tests for StatusEffectBase duration management and tag system.
 */
#include "Core/EffectIds.h"
#include "Core/StatusEffectBase.h"
#include "doctest.h"
#include <ostream>

namespace
{
// Minimal concrete subclass for testing the base in isolation.
class NoOpEffect : public StatusEffectBase
{
  public:
    explicit NoOpEffect(std::optional<int> dur)
        : StatusEffectBase{EffectIds::kBurn, "NoOp", dur, {EffectTags::kDebuff, EffectTags::kDoT}}
    {
    }
    bool isBuff() const override
    {
        return false;
    }
    bool isDebuff() const override
    {
        return true;
    }
};
} // namespace

TEST_CASE("StatusEffectBase: timed duration decrements via extendDuration(-1)")
{
    NoOpEffect e{std::make_optional(3)};
    REQUIRE(*e.getDuration() == 3);

    e.extendDuration(-1);
    CHECK(*e.getDuration() == 2);
    e.extendDuration(-1);
    CHECK(*e.getDuration() == 1);
    e.extendDuration(-1);
    CHECK(*e.getDuration() == 0);

    // Must clamp at 0, never go negative.
    e.extendDuration(-1);
    CHECK(*e.getDuration() == 0);
}

TEST_CASE("StatusEffectBase: permanent effect (nullopt) ignores extendDuration")
{
    NoOpEffect e{std::nullopt};
    REQUIRE(!e.getDuration().has_value());

    e.extendDuration(-1);
    CHECK(!e.getDuration().has_value());

    e.extendDuration(5);
    CHECK(!e.getDuration().has_value());
}

TEST_CASE("StatusEffectBase: extendDuration adds turns correctly")
{
    NoOpEffect e{std::make_optional(2)};
    e.extendDuration(3);
    CHECK(*e.getDuration() == 5);
}

TEST_CASE("StatusEffectBase: hasTag matches declared tags, rejects absent tags")
{
    NoOpEffect e{std::nullopt};
    CHECK(e.hasTag(EffectTags::kDebuff));
    CHECK(e.hasTag(EffectTags::kDoT));
    CHECK(!e.hasTag(EffectTags::kBuff));
    CHECK(!e.hasTag("nonexistent"));
}

TEST_CASE("StatusEffectBase: getId and getName return correct values")
{
    NoOpEffect e{std::nullopt};
    CHECK(e.getId() == EffectIds::kBurn);
    CHECK(e.getName() == "NoOp");
}

TEST_CASE("StatusEffectBase: default absorbDamage returns incoming unchanged")
{
    NoOpEffect e{std::nullopt};
    CHECK(e.absorbDamage(50) == 50);
    CHECK(e.absorbDamage(0) == 0);
}

TEST_CASE("StatusEffectBase: default isExhausted is false")
{
    NoOpEffect e{std::nullopt};
    CHECK(!e.isExhausted());
}

TEST_CASE("StatusEffectBase: default modifyStatsFlat returns stats unchanged")
{
    NoOpEffect e{std::nullopt};
    Stats base{100, 100, 15, 5, 10};
    Stats result = e.modifyStatsFlat(base);
    CHECK(result.hp == base.hp);
    CHECK(result.atk == base.atk);
    CHECK(result.def == base.def);
    CHECK(result.spd == base.spd);
}

TEST_CASE("StatusEffectBase: default modifyStatsPct returns stats unchanged")
{
    NoOpEffect e{std::nullopt};
    Stats base{100, 100, 15, 5, 10};
    Stats result = e.modifyStatsPct(base);
    CHECK(result.hp == base.hp);
    CHECK(result.spd == base.spd);
}