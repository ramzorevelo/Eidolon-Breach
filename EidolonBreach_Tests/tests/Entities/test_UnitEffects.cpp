/**
 * @file test_UnitEffects.cpp
 * @brief Tests for Unit effect management, takeTrueDamage, and shield absorption.
 *
 * Integration tests that require concrete effects (getFinalStats with SlowEffect,
 * tickEffects with Burn/Regen) are in test_UnitEffectsIntegration.cpp.
 */
#include "Core/EffectIds.h"
#include "Core/StatusEffectBase.h"
#include "Core/Effects/BurnEffect.h"
#include "Core/Effects/RegenEffect.h"
#include "Core/Effects/ShieldEffect.h"
#include "Core/Effects/SlowEffect.h"
#include "doctest.h"
#include "test_helpers.h"
#include <memory>
#include <ostream>

namespace
{
// Minimal effect that absorbs a fixed amount of direct damage.
class FixedShieldEffect : public StatusEffectBase
{
  public:
    explicit FixedShieldEffect(int absorb, int dur = 3)
        : StatusEffectBase{EffectIds::kShield, "TestShield", std::make_optional(dur), {EffectTags::kBuff, EffectTags::kShield}}, m_remaining{absorb}
    {
    }
    bool isBuff() const override
    {
        return true;
    }
    bool isDebuff() const override
    {
        return false;
    }
    int absorbDamage(int incoming) override
    {
        int absorbed{std::min(incoming, m_remaining)};
        m_remaining -= absorbed;
        return incoming - absorbed;
    }
    bool isExhausted() const override
    {
        return m_remaining <= 0;
    }

  private:
    int m_remaining;
};

// No-op effect with a configurable tick count.
class CountingEffect : public StatusEffectBase
{
  public:
    explicit CountingEffect(int dur)
        : StatusEffectBase{EffectIds::kBurn, "Counter", std::make_optional(dur), {EffectTags::kDebuff, EffectTags::kDoT}}
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
    std::string onTick(Unit & /*owner*/) override
    {
        ++tickCount;
        return "ticked";
    }
    int tickCount{0};
};
} // namespace

// ── takeTrueDamage ────────────────────────────────────────────────────────────

TEST_CASE("Unit::takeTrueDamage: reduces HP directly, ignoring shields")
{
    auto hero = makeHero(); // 120 HP
    hero->applyEffect(std::make_unique<FixedShieldEffect>(100, 3));
    hero->takeTrueDamage(30);
    // Shield must NOT absorb takeTrueDamage — HP drops directly.
    CHECK(hero->getHp() == 90);
}

TEST_CASE("Unit::takeTrueDamage: clamps HP to 0, does not go negative")
{
    auto hero = makeHero();
    hero->takeTrueDamage(9999);
    CHECK(hero->getHp() == 0);
    CHECK(!hero->isAlive());
}

// ── takeDamage: shield absorption ────────────────────────────────────────────

TEST_CASE("Unit::takeDamage: shield absorbs damage fully when pool is sufficient")
{
    auto hero = makeHero(); // 120 HP
    hero->applyEffect(std::make_unique<FixedShieldEffect>(50, 3));
    hero->takeDamage(30);
    CHECK(hero->getHp() == 120); // shield absorbed all 30
}

TEST_CASE("Unit::takeDamage: overflow hits HP when shield pool is insufficient")
{
    auto hero = makeHero(); // 120 HP
    hero->applyEffect(std::make_unique<FixedShieldEffect>(20, 3));
    hero->takeDamage(50);
    // Shield absorbs 20, 30 overflows to HP: 120 - 30 = 90.
    CHECK(hero->getHp() == 90);
}

TEST_CASE("Unit::takeDamage: no shield applies full damage to HP")
{
    auto hero = makeHero(); // 120 HP
    hero->takeDamage(40);
    CHECK(hero->getHp() == 80);
}

// ── applyEffect: refresh semantics ───────────────────────────────────────────

TEST_CASE("Unit::applyEffect: adds a new effect")
{
    auto hero = makeHero();
    hero->applyEffect(std::make_unique<FixedShieldEffect>(30, 3));
    CHECK(hero->hasEffect(EffectIds::kShield));
}

TEST_CASE("Unit::applyEffect: same ID replaces existing effect (refresh semantics)")
{
    auto hero = makeHero();
    hero->applyEffect(std::make_unique<FixedShieldEffect>(30, 3));
    hero->applyEffect(std::make_unique<FixedShieldEffect>(80, 3)); // replace

    // Only one shield effect must exist.
    int count{0};
    for (const auto &e : hero->getEffects())
        if (e->getId() == EffectIds::kShield)
            ++count;
    CHECK(count == 1);

    // New shield absorbs 80, not 30.
    hero->takeDamage(60);
    CHECK(hero->getHp() == 120); // all absorbed by new 80-shield
}

TEST_CASE("Unit::applyEffect: different IDs coexist")
{
    auto hero = makeHero();
    hero->applyEffect(std::make_unique<FixedShieldEffect>(30, 3));
    hero->applyEffect(std::make_unique<CountingEffect>(2));
    CHECK(hero->hasEffect(EffectIds::kShield));
    CHECK(hero->hasEffect(EffectIds::kBurn));
}

// ── removeEffect / removeEffectsByTag ────────────────────────────────────────

TEST_CASE("Unit::removeEffect: removes effect by ID")
{
    auto hero = makeHero();
    hero->applyEffect(std::make_unique<FixedShieldEffect>(30, 3));
    REQUIRE(hero->hasEffect(EffectIds::kShield));
    hero->removeEffect(EffectIds::kShield);
    CHECK(!hero->hasEffect(EffectIds::kShield));
}

TEST_CASE("Unit::removeEffect: no-op when ID is not present")
{
    auto hero = makeHero();
    hero->removeEffect(EffectIds::kBurn); // must not crash
    CHECK(!hero->hasEffect(EffectIds::kBurn));
}

TEST_CASE("Unit::removeEffectsByTag: removes all effects with matching tag")
{
    auto hero = makeHero();
    hero->applyEffect(std::make_unique<FixedShieldEffect>(30, 3));
    hero->applyEffect(std::make_unique<CountingEffect>(2));
    // CountingEffect has kDebuff and kDoT tags. FixedShieldEffect has kBuff and kShield.
    hero->removeEffectsByTag(EffectTags::kDebuff);
    CHECK(!hero->hasEffect(EffectIds::kBurn));
    CHECK(hero->hasEffect(EffectIds::kShield)); // Buff, not Debuff — must survive
}

TEST_CASE("Unit::removeEffectsByTag: no-op when no effects match")
{
    auto hero = makeHero();
    hero->applyEffect(std::make_unique<FixedShieldEffect>(30, 3));
    hero->removeEffectsByTag(EffectTags::kDebuff); // no debuffs present
    CHECK(hero->hasEffect(EffectIds::kShield));
}

// ── hasEffectWithTag ─────────────────────────────────────────────────────────

TEST_CASE("Unit::hasEffectWithTag: returns true when a matching tag is active")
{
    auto hero = makeHero();
    CHECK(!hero->hasEffectWithTag(EffectTags::kDoT));
    hero->applyEffect(std::make_unique<CountingEffect>(2));
    CHECK(hero->hasEffectWithTag(EffectTags::kDoT));
}

// ── tickEffects ──────────────────────────────────────────────────────────────

TEST_CASE("Unit::tickEffects: calls onTick and returns non-empty messages")
{
    auto hero = makeHero();
    auto *rawEffect = new CountingEffect(2);
    hero->applyEffect(std::unique_ptr<CountingEffect>(rawEffect));

    auto msgs = hero->tickEffects();
    CHECK(rawEffect->tickCount == 1);
    CHECK(msgs.size() == 1);
    CHECK(msgs[0] == "ticked");
}

TEST_CASE("Unit::tickEffects: decrements duration each call")
{
    auto hero = makeHero();
    hero->applyEffect(std::make_unique<CountingEffect>(3));

    hero->tickEffects();
    REQUIRE(hero->hasEffect(EffectIds::kBurn));
    CHECK(*hero->getEffects()[0]->getDuration() == 2);

    hero->tickEffects();
    CHECK(*hero->getEffects()[0]->getDuration() == 1);
}

TEST_CASE("Unit::tickEffects: removes effect when duration reaches 0")
{
    auto hero = makeHero();
    hero->applyEffect(std::make_unique<CountingEffect>(1)); // expires after 1 tick
    REQUIRE(hero->hasEffect(EffectIds::kBurn));

    hero->tickEffects();
    CHECK(!hero->hasEffect(EffectIds::kBurn));
}

TEST_CASE("Unit::tickEffects: removes exhausted shield after next tick")
{
    auto hero = makeHero();
    hero->applyEffect(std::make_unique<FixedShieldEffect>(10, 3));
    hero->takeDamage(20); // depletes shield (absorbs 10, 10 overflows)
    REQUIRE(hero->getEffects()[0]->isExhausted());

    hero->tickEffects(); // cleanup pass
    CHECK(!hero->hasEffect(EffectIds::kShield));
}

TEST_CASE("Unit::tickEffects: permanent effect never expires")
{
    auto hero = makeHero();
    // Construct permanent shield by passing nullopt duration through StatusEffectBase.
    class PermanentShield : public StatusEffectBase
    {
      public:
        PermanentShield()
            : StatusEffectBase{EffectIds::kShield, "PermShield", std::nullopt, {EffectTags::kBuff, EffectTags::kShield}}
        {
        }
        bool isBuff() const override
        {
            return true;
        }
        bool isDebuff() const override
        {
            return false;
        }
        int absorbDamage(int incoming) override
        {
            return incoming;
        }
        bool isExhausted() const override
        {
            return false;
        }
    };
    hero->applyEffect(std::make_unique<PermanentShield>());

    for (int i{0}; i < 10; ++i)
        hero->tickEffects();

    CHECK(hero->hasEffect(EffectIds::kShield));
}

// ── extendEffectsByTag ───────────────────────────────────────────────────────

TEST_CASE("Unit::extendEffectsByTag: extends duration of matching effects")
{
    auto hero = makeHero();
    hero->applyEffect(std::make_unique<CountingEffect>(2));
    hero->extendEffectsByTag(EffectTags::kDoT, 3);
    CHECK(*hero->getEffects()[0]->getDuration() == 5);
}

TEST_CASE("Unit::extendEffectsByTag: non-matching effects are unaffected")
{
    auto hero = makeHero();
    hero->applyEffect(std::make_unique<CountingEffect>(2));
    hero->extendEffectsByTag(EffectTags::kBuff, 3); // no buff effects
    CHECK(*hero->getEffects()[0]->getDuration() == 2);
}

TEST_CASE("Unit::getFinalStats two-pass: SlowEffect reduces SPD in final stats only")
{
    auto hero = makeHero(); // SPD 10
    hero->applyEffect(std::make_unique<SlowEffect>(0.30f, 2));
    CHECK(hero->getFinalStats().spd == 7);
    CHECK(hero->getBaseStats().spd == 10);
}

TEST_CASE("Unit::tickEffects: BurnEffect deals damage via takeTrueDamage")
{
    auto hero = makeHero(); // 120 HP
    hero->applyEffect(std::make_unique<BurnEffect>(15, 2));
    hero->tickEffects();
    CHECK(hero->getHp() == 105); // 120 - 15
}

TEST_CASE("Unit::tickEffects: BurnEffect bypasses ShieldEffect")
{
    auto hero = makeHero(); // 120 HP
    hero->applyEffect(std::make_unique<ShieldEffect>(100, 3));
    hero->applyEffect(std::make_unique<BurnEffect>(20, 2));
    hero->tickEffects();
    // Burn calls takeTrueDamage — shield untouched, HP drops.
    CHECK(hero->getHp() == 100);
}

TEST_CASE("Unit::tickEffects: RegenEffect heals HP on tick")
{
    auto hero = makeHero();   // 120 HP
    hero->takeTrueDamage(40); // 80 HP
    hero->applyEffect(std::make_unique<RegenEffect>(10, 2));
    hero->tickEffects();
    CHECK(hero->getHp() == 90);
}

TEST_CASE("Unit::tickEffects: multiple effects both fire and produce messages")
{
    auto hero = makeHero();   // 120 HP
    hero->takeTrueDamage(50); // 70 HP
    hero->applyEffect(std::make_unique<BurnEffect>(10, 2));
    hero->applyEffect(std::make_unique<RegenEffect>(5, 2));

    auto msgs = hero->tickEffects();
    // Burn costs 10 HP (true damage), Regen heals 5 HP: net 70 - 10 + 5 = 65
    CHECK(hero->getHp() == 65);
    CHECK(msgs.size() == 2);
}

TEST_CASE("Unit::tickEffects: ShieldEffect expired by duration is removed")
{
    auto hero = makeHero();
    hero->applyEffect(std::make_unique<ShieldEffect>(50, 1)); // expires after 1 tick
    REQUIRE(hero->hasEffect(EffectIds::kShield));
    hero->tickEffects();
    CHECK(!hero->hasEffect(EffectIds::kShield));
}

TEST_CASE("Unit: removeEffectsByTag cleanses Debuffs, leaves Buffs")
{
    auto hero = makeHero();
    hero->applyEffect(std::make_unique<BurnEffect>(5, 2));
    hero->applyEffect(std::make_unique<SlowEffect>(0.30f, 2));
    hero->applyEffect(std::make_unique<ShieldEffect>(50, 3));

    hero->removeEffectsByTag(EffectTags::kDebuff);

    CHECK(!hero->hasEffect(EffectIds::kBurn));
    CHECK(!hero->hasEffect(EffectIds::kSlow));
    CHECK(hero->hasEffect(EffectIds::kShield));
}

TEST_CASE("Unit: extendEffectsByTag extends HoT effects")
{
    auto hero = makeHero();
    hero->applyEffect(std::make_unique<RegenEffect>(10, 2));
    hero->extendEffectsByTag(EffectTags::kHoT, 3);
    CHECK(*hero->getEffects()[0]->getDuration() == 5);
}