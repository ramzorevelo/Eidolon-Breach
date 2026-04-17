/**
 * @file test_RegenEffect.cpp
 * @brief Tests for RegenEffect HP restoration on tick.
 */
#include "Core/EffectIds.h"
#include "Core/Effects/RegenEffect.h"
#include "doctest.h"
#include "test_helpers.h"
#include <ostream>

TEST_CASE("RegenEffect: heals correct amount on tick")
{
    auto hero = makeHero();   // 120/120 HP
    hero->takeTrueDamage(50); // bring to 70 HP
    RegenEffect regen{15, 2};
    regen.onTick(*hero);
    CHECK(hero->getHp() == 85);
}

TEST_CASE("RegenEffect: heal is capped at maxHp")
{
    auto hero = makeHero(); // already at 120/120
    RegenEffect regen{20, 2};
    regen.onTick(*hero);
    CHECK(hero->getHp() == 120);
}

TEST_CASE("RegenEffect: onTick returns a non-empty message")
{
    auto hero = makeHero();
    hero->takeTrueDamage(40);
    RegenEffect regen{10, 2};
    std::string msg = regen.onTick(*hero);
    CHECK(!msg.empty());
}

TEST_CASE("RegenEffect: correct ID, tags, and categorisation")
{
    RegenEffect regen{};
    CHECK(regen.getId() == EffectIds::kRegen);
    CHECK(regen.isBuff() == true);
    CHECK(regen.isDebuff() == false);
    CHECK(regen.hasTag(EffectTags::kHoT));
    CHECK(regen.hasTag(EffectTags::kTerra));
    CHECK(regen.hasTag(EffectTags::kBuff));
}

TEST_CASE("RegenEffect: does not absorb direct damage")
{
    RegenEffect regen{10, 2};
    CHECK(regen.absorbDamage(50) == 50);
}