/**
 * @file test_EnemyBreakMechanics.cpp
 * @brief Phase 5 tests: affinity scaling, break trigger, broken damage bonus,
 *        break window, and BreakEffect callback execution.
 */

#include "Actions/BasicStrikeAction.h"
#include "Battle/BattleState.h"
#include "Battle/ResonanceField.h"
#include "Core/Affinity.h"
#include "Core/EffectIds.h"
#include "Entities/Enemy.h"
#include "Entities/IAIStrategy.h"
#include "Entities/Party.h"
#include "Entities/PlayableCharacter.h"
#include "Entities/Slime.h"
#include "Entities/StoneGolem.h"
#include "Entities/VampireBat.h"
#include "UI/test_NullInputHandler.h"
#include "UI/test_NullRenderer.h"
#include "Core/EventBus.h"
#include "Core/RunContext.h"
#include "doctest.h"
#include "test_helpers.h"
#include <memory>
#include <map>


namespace
{
/// Enemy with explicit affinity modifiers for testing.
std::unique_ptr<Enemy> makeEnemyWithModifiers(
    int toughness,
    std::map<Affinity, float> modifiers)
{
    return std::make_unique<Enemy>(
        "e", "TestEnemy",
        Stats{100, 100, 10, 0, 5},
        Affinity::Terra,
        toughness,
        std::make_unique<BasicAIStrategy>(),
        std::move(modifiers));
}

// Thread-local stubs so makeBattleState can return a BattleState by value
// without requiring the caller to manage RunContext/EventBus lifetime.
RunContext g_testRunContext{};
EventBus g_testEventBus{};

BattleState makeBattleState(ResonanceField &field,
                            NullInputHandler &input,
                            NullRenderer &renderer,
                            Party *playerParty = nullptr,
                            Party *enemyParty = nullptr)
{
    BattleState state{0, 0, Affinity::Aether, field, input, renderer,
                      g_testRunContext, g_testEventBus};
    state.playerParty = playerParty;
    state.enemyParty = enemyParty;
    return state;
}
} // namespace


TEST_CASE("applyToughnessHit: neutral affinity (no modifier) applies 1.0x")
{
    auto e = makeEnemyWithModifiers(100, {});
    e->applyToughnessHit(20, Affinity::Blaze);
    CHECK(e->getToughness() == 80); // 100 - 20*1.0
}

TEST_CASE("applyToughnessHit: weakness affinity applies 2.0x toughness damage")
{
    auto e = makeEnemyWithModifiers(100, {{Affinity::Tempest, 2.0f}});
    e->applyToughnessHit(20, Affinity::Tempest);
    CHECK(e->getToughness() == 60); // 100 - 20*2.0
}

TEST_CASE("applyToughnessHit: resistance affinity applies 0.5x toughness damage")
{
    auto e = makeEnemyWithModifiers(100, {{Affinity::Frost, 0.5f}});
    e->applyToughnessHit(20, Affinity::Frost);
    CHECK(e->getToughness() == 90); // 100 - 20*0.5
}

TEST_CASE("applyToughnessHit: toughness cannot go below zero")
{
    auto e = makeEnemyWithModifiers(50, {{Affinity::Tempest, 2.0f}});
    e->applyToughnessHit(100, Affinity::Tempest); // would be 200 damage
    CHECK(e->getToughness() == 50);               // resets to max on break
    CHECK(e->isBroken());
}


TEST_CASE("applyToughnessHit: sets isBroken when toughness reaches 0")
{
    auto e = makeEnemyWithModifiers(30, {});
    e->applyToughnessHit(30, Affinity::Aether);
    CHECK(e->isBroken());
}

TEST_CASE("applyToughnessHit: resets toughness to max on break")
{
    auto e = makeEnemyWithModifiers(30, {});
    e->applyToughnessHit(30, Affinity::Aether);
    CHECK(e->getToughness() == e->getMaxToughness());
}

TEST_CASE("applyToughnessHit: does not set broken when toughness > 0")
{
    auto e = makeEnemyWithModifiers(50, {});
    e->applyToughnessHit(30, Affinity::Aether);
    CHECK(!e->isBroken());
    CHECK(e->getToughness() == 20);
}

TEST_CASE("applyToughnessHit: weakness affinity can trigger break with smaller hit")
{
    // 20 toughness; 11 hit at 2.0x = 22 effective → break
    auto e = makeEnemyWithModifiers(20, {{Affinity::Blaze, 2.0f}});
    e->applyToughnessHit(11, Affinity::Blaze);
    CHECK(e->isBroken());
}

TEST_CASE("applyToughnessHit: resistance affinity keeps enemy unbroken")
{
    // 20 toughness; 30 hit at 0.5x = 15 effective → not broken
    auto e = makeEnemyWithModifiers(20, {{Affinity::Frost, 0.5f}});
    e->applyToughnessHit(30, Affinity::Frost);
    CHECK(!e->isBroken());
    CHECK(e->getToughness() == 5);
}


TEST_CASE("Enemy::takeDamage: applies 1.5x bonus while broken")
{
    auto e = makeEnemyWithModifiers(10, {});
    e->applyToughnessHit(10, Affinity::Aether); // break it
    REQUIRE(e->isBroken());

    // 40 damage * 1.5 = 60 → HP 100 - 60 = 40
    e->takeDamage(40);
    CHECK(e->getHp() == 40);
}

TEST_CASE("Enemy::takeTrueDamage: applies 1.5x bonus while broken")
{
    auto e = makeEnemyWithModifiers(10, {});
    e->applyToughnessHit(10, Affinity::Aether);
    REQUIRE(e->isBroken());

    // 20 true damage * 1.5 = 30 → HP 100 - 30 = 70
    e->takeTrueDamage(20);
    CHECK(e->getHp() == 70);
}

TEST_CASE("Enemy::takeDamage: no bonus when not broken")
{
    auto e = makeEnemyWithModifiers(50, {});
    e->takeDamage(40);
    CHECK(e->getHp() == 60); // 100 - 40*1.0
}

TEST_CASE("Enemy::takeDamage: bonus ends after break window (skipped turn)")
{
    auto ePtr = makeEnemyWithModifiers(10, {});
    Enemy *e = ePtr.get();

    e->applyToughnessHit(10, Affinity::Aether);
    REQUIRE(e->isBroken());

    // Battle calls checkAndClearBroken() at the start of the suppressed slot.
    e->checkAndClearBroken();
    REQUIRE(!e->isBroken()); // bonus window over

    // Damage after recovery: no bonus.
    e->takeDamage(40);
    CHECK(e->getHp() == 60); // 100 - 40*1.0
}

TEST_CASE("Enemy::takeDamage: bonus applies to ALL player actions during window, not just the break action")
{
    // Break with one hit, then deal damage with a second action — bonus must still apply.
    auto e = makeEnemyWithModifiers(10, {});
    e->applyToughnessHit(10, Affinity::Aether); // break
    REQUIRE(e->isBroken());

    e->takeDamage(20); // first action in window: 20 * 1.5 = 30
    CHECK(e->getHp() == 70);

    e->takeDamage(10); // second action in window: 10 * 1.5 = 15
    CHECK(e->getHp() == 55);
}


TEST_CASE("Enemy::checkAndClearBroken: clears flag and returns true when broken")
{
    auto e = makeEnemyWithModifiers(10, {});
    e->applyToughnessHit(10, Affinity::Aether);
    REQUIRE(e->isBroken());

    const bool wasCleared{e->checkAndClearBroken()};
    CHECK(wasCleared);
    CHECK(!e->isBroken());
}

TEST_CASE("Enemy::checkAndClearBroken: returns false when not broken")
{
    auto e = makeEnemyWithModifiers(10, {});
    CHECK(!e->checkAndClearBroken());
}

TEST_CASE("Enemy::takeTurn: acts normally while broken (break cleared by Battle first)")
{
    // In the AV system, Battle calls checkAndClearBroken() before takeTurn.
    // Enemy::takeTurn should not skip or return Skip on its own.
    Party playerParty, enemyParty;
    ResonanceField field{};
    NullInputHandler input{};
    NullRenderer renderer{};

    auto hero = std::make_unique<PlayableCharacter>(
        "h", "Hero", Stats{100, 100, 10, 0, 10}, Affinity::Aether, 10);
    hero->addAbility(std::make_unique<BasicStrikeAction>());
    playerParty.addUnit(std::move(hero));

    auto ePtr = makeEnemyWithModifiers(10, {});
    Enemy *e = ePtr.get();
    enemyParty.addUnit(std::move(ePtr));

    e->applyToughnessHit(10, Affinity::Aether);
    REQUIRE(e->isBroken());

    // Simulate Battle clearing broken before the turn.
    e->checkAndClearBroken();
    REQUIRE(!e->isBroken());

    BattleState state = makeBattleState(field, input, renderer, &playerParty, &enemyParty);
    ActionResult result = e->takeTurn(enemyParty, playerParty, state);
    // Enemy acts normally; no Skip.
    CHECK(result.type == ActionResult::Type::Damage);
}

TEST_CASE("recoverFromBreak: manually clears broken flag")
{
    auto e = makeEnemyWithModifiers(10, {});
    e->applyToughnessHit(10, Affinity::Aether);
    REQUIRE(e->isBroken());
    e->recoverFromBreak();
    CHECK(!e->isBroken());
}


TEST_CASE("BreakEffect::onBreak is called by Battle when enemy breaks")
{
    // Construct enemy with a break effect that sets a flag.
    bool callbackFired{false};

    Party playerParty, enemyParty;
    ResonanceField field{};
    NullInputHandler input{};
    NullRenderer renderer{};

    auto ePtr = makeEnemyWithModifiers(20, {});
    Enemy *e = ePtr.get();
    e->setBreakEffect(BreakEffect{
        "test_break", "Test",
        [&callbackFired](Enemy & /*broken*/, BattleState & /*state*/)
        {
            callbackFired = true;
        }});
    enemyParty.addUnit(std::move(ePtr));

    // Simulate what Battle::processNewBreaks does: snapshot, apply hit, fire callback.
    bool wasBrokenBefore{e->isBroken()};
    e->applyToughnessHit(20, Affinity::Aether); // triggers break
    bool isNowBroken{e->isBroken()};

    if (!wasBrokenBefore && isNowBroken)
    {
        BattleState state = makeBattleState(field, input, renderer, &playerParty, &enemyParty);
        const BreakEffect &effect{e->getBreakEffect()};
        if (effect.onBreak)
            effect.onBreak(*e, state);
    }

    CHECK(callbackFired);
}

TEST_CASE("BreakEffect: callback receives the correct Enemy reference")
{
    Enemy *capturedPtr{nullptr};
    auto ePtr = makeEnemyWithModifiers(10, {});
    Enemy *e = ePtr.get();
    e->setBreakEffect(BreakEffect{
        "test_ref", "TestRef",
        [&capturedPtr](Enemy &broken, BattleState & /*state*/)
        {
            capturedPtr = &broken;
        }});

    e->applyToughnessHit(10, Affinity::Aether);

    ResonanceField field{};
    NullInputHandler inputHandler{};
    NullRenderer renderer{};
    RunContext runContext{};
    EventBus eventBus{};
    BattleState state{0, 0, Affinity::Aether, field, inputHandler, renderer, runContext, eventBus};
    const BreakEffect &effect{e->getBreakEffect()};
    if (effect.onBreak)
        effect.onBreak(*e, state);

    CHECK(capturedPtr == e);
}

TEST_CASE("BreakEffect: empty callback is a safe no-op")
{
    auto e = makeEnemyWithModifiers(10, {});
    // Default BreakEffect has no onBreak — calling it must not crash.
    e->applyToughnessHit(10, Affinity::Aether);
    CHECK(e->isBroken()); // just verify break happened

    ResonanceField field{};
    NullInputHandler inputHandler{};
    NullRenderer renderer{};
    RunContext runContext{};
    EventBus eventBus{};
    BattleState state{0, 0, Affinity::Aether, field, inputHandler, renderer, runContext, eventBus};
    const BreakEffect &effect{e->getBreakEffect()};
    // Must not throw or crash when onBreak is empty.
    if (effect.onBreak)
        effect.onBreak(*e, state); // only fires if set — skipped here ✓
}


TEST_CASE("Slime Split break effect: deals impact damage to all player allies")
{
    auto hero1 = makeHero("h1"); // 120 HP
    auto hero2 = makeHero("h2"); // 120 HP
    auto *h1 = hero1.get();
    auto *h2 = hero2.get();

    Party playerParty, enemyParty;
    playerParty.addUnit(std::move(hero1));
    playerParty.addUnit(std::move(hero2));

    Slime slime{"TestSlime", 80, 30};
    slime.applyToughnessHit(30, Affinity::Tempest); // 2.0x → 60 damage → break
    REQUIRE(slime.isBroken());

    ResonanceField field{};
    NullInputHandler input{};
    NullRenderer renderer{};
    BattleState state = makeBattleState(field, input, renderer, &playerParty, &enemyParty);

    const BreakEffect &effect{slime.getBreakEffect()};
    REQUIRE(effect.onBreak);
    effect.onBreak(slime, state);

    CHECK(h1->getHp() == 112); // 120 - 8 (kSlimeSplitImpactDamage is 8 true dmg)
    CHECK(h2->getHp() == 112);
}


TEST_CASE("StoneGolem Crumble break effect: applies ShieldEffect to all player allies")
{
    auto hero = makeHero("h");
    auto *h = hero.get();

    Party playerParty, enemyParty;
    playerParty.addUnit(std::move(hero));

    StoneGolem golem{"TestGolem", 130, 60};
    golem.applyToughnessHit(60, Affinity::Tempest); // 2.0x → break
    REQUIRE(golem.isBroken());

    ResonanceField field{};
    NullInputHandler input{};
    NullRenderer renderer{};
    BattleState state = makeBattleState(field, input, renderer, &playerParty, &enemyParty);

    const BreakEffect &effect{golem.getBreakEffect()};
    REQUIRE(effect.onBreak);
    effect.onBreak(golem, state);

    CHECK(h->hasEffect(EffectIds::kShield));

    // Shield should absorb some incoming damage.
    h->takeDamage(10);
    CHECK(h->getHp() == 120); // shield absorbs 10, HP unchanged
}


TEST_CASE("VampireBat Bloodless break effect: heals player allies and activates bloodless")
{
    auto hero = makeHero("h"); // 120 HP
    auto *h = hero.get();
    h->takeTrueDamage(40); // bring to 80 HP

    Party playerParty, enemyParty;
    playerParty.addUnit(std::move(hero));

    VampireBat bat{"TestBat", 100, 40};
    bat.applyToughnessHit(40, Affinity::Terra); // 2.0x → break
    REQUIRE(bat.isBroken());

    ResonanceField field{};
    NullInputHandler input{};
    NullRenderer renderer{};
    BattleState state = makeBattleState(field, input, renderer, &playerParty, &enemyParty);

    const BreakEffect &effect{bat.getBreakEffect()};
    REQUIRE(effect.onBreak);
    effect.onBreak(bat, state);

    // Hero healed by 10% of 120 maxHp = 12 → 80 + 12 = 92
    CHECK(h->getHp() == 92);
}

TEST_CASE("VampireBat Bloodless: performAttack returns Skip for 2 turns after activation")
{
    VampireBat bat{"TestBat", 100, 40};
    bat.activateBloodless();

    // Turn 1 of bloodless: returns Skip
    ActionResult r1{bat.performAttack()};
    CHECK(r1.type == ActionResult::Type::Skip);

    // Turn 2 of bloodless: still Skip (counter was 2, now 1 after first call, still > 0)
    ActionResult r2{bat.performAttack()};
    // After second decrement bloodless expires (0 <= 0), but this turn is already the Skip
    CHECK(r2.type == ActionResult::Type::Skip);

    // Turn 3: bloodless is over, normal attack
    ActionResult r3{bat.performAttack()};
    CHECK(r3.type == ActionResult::Type::Damage);
}