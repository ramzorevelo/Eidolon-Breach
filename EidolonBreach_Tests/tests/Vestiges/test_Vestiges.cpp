/**
 * @file test_Vestiges.cpp
 * @brief Unit tests for all five Phase 7 vestiges and Party vestige limit.
 */
#include "Battle/BattleState.h"
#include "Battle/ResonanceField.h"
#include "Core/ActionResult.h"
#include "Core/BattleEvents.h"
#include "Core/EventBus.h"
#include "Core/RunContext.h"
#include "Entities/Party.h"
#include "Vestiges/AttunistGambitVestige.h"
#include "Vestiges/EchoingStrikeVestige.h"
#include "Vestiges/FlameResonanceVestige.h"
#include "Vestiges/ToughnessBreakerVestige.h"
#include "Vestiges/VestigeOfTheUnbound.h"
#include "UI/test_NullInputHandler.h"
#include "UI/test_NullRenderer.h"
#include "doctest.h"
#include "test_helpers.h"
#include <memory>
#include <ostream>

// ── helpers ──────────────────────────────────────────────────────────────────

namespace
{
// Constructs a minimal BattleState pointing at real field/input/renderer stubs.


ResonanceField g_field{};
NullInputHandler g_input{};
NullRenderer g_renderer{};
RunContext g_runCtx{};
EventBus g_eventBus{};

BattleState makeState(Party *playerParty = nullptr, Party *enemyParty = nullptr)
{
    g_field = ResonanceField{};
    g_eventBus = EventBus{};
    BattleState state{0, 0, Affinity::Aether, g_field, g_input, g_renderer,
                      g_runCtx, g_eventBus};
    state.playerParty = playerParty;
    state.enemyParty = enemyParty;
    return state;
}
} // namespace

// ── FlameResonanceVestige ────────────────────────────────────────────────────

TEST_CASE("FlameResonanceVestige: Blaze action adds kBlazeBonus to Resonance Field")
{
    FlameResonanceVestige vestige{};
    auto hero = makeHero();
    BattleState state{makeState()};

    ActionResult result{};
    result.actionAffinity = Affinity::Blaze;
    vestige.onAction(*hero, result, state);

    CHECK(state.resonanceField.getGauge() == 5);
}

TEST_CASE("FlameResonanceVestige: non-Blaze action does not modify Resonance Field")
{
    FlameResonanceVestige vestige{};
    auto hero = makeHero();
    BattleState state{makeState()};

    ActionResult result{};
    result.actionAffinity = Affinity::Frost;
    vestige.onAction(*hero, result, state);

    CHECK(state.resonanceField.getGauge() == 0);
}

// ── ToughnessBreakerVestige ──────────────────────────────────────────────────

TEST_CASE("ToughnessBreakerVestige: applies bonus toughness against weakness affinity")
{
    ToughnessBreakerVestige vestige{};

    // Enemy with 2.0x modifier for Tempest (weakness).
    auto ePtr = std::make_unique<Enemy>(
        "e", "WeakEnemy",
        Stats{100, 100, 10, 0, 5},
        Affinity::Frost,
        50,
        std::make_unique<BasicAIStrategy>(),
        std::map<Affinity, float>{{Affinity::Tempest, 2.0f}});
    Enemy *e = ePtr.get();

    Party enemyParty;
    enemyParty.addUnit(std::move(ePtr));
    BattleState state{makeState(nullptr, &enemyParty)};

    // Simulate action that already applied 10 base toughness damage externally.
    e->applyToughnessHit(10, Affinity::Tempest); // base: 10 * 2.0 = 20 removed → 30 left
    REQUIRE(e->getToughness() == 30);

    ActionResult result{};
    result.actionAffinity = Affinity::Tempest;
    result.toughnessDamage = 10;
    result.targetEnemyIndex = 0;

    auto hero = makeHero();
    vestige.onAction(*hero, result, state);

    // Bonus: 10 * 0.2 * 2.0 = 4 more → 30 - 4 = 26
    CHECK(e->getToughness() == 26);
}

TEST_CASE("ToughnessBreakerVestige: no bonus against neutral affinity")
{
    ToughnessBreakerVestige vestige{};

    // Enemy with no affinity modifiers (all neutral 1.0x).
    auto ePtr = makeEnemy(100, 50);
    Enemy *e = ePtr.get();

    Party enemyParty;
    enemyParty.addUnit(std::move(ePtr));
    BattleState state{makeState(nullptr, &enemyParty)};

    e->applyToughnessHit(10, Affinity::Blaze); // 10 * 1.0 = 10 removed → 40 left
    REQUIRE(e->getToughness() == 40);

    ActionResult result{};
    result.actionAffinity = Affinity::Blaze;
    result.toughnessDamage = 10;
    result.targetEnemyIndex = 0;

    auto hero = makeHero();
    vestige.onAction(*hero, result, state);

    // No bonus — toughness unchanged.
    CHECK(e->getToughness() == 40);
}

TEST_CASE("ToughnessBreakerVestige: no-op when toughnessDamage is 0")
{
    ToughnessBreakerVestige vestige{};
    auto ePtr = makeEnemy(100, 50);
    Enemy *e = ePtr.get();

    Party enemyParty;
    enemyParty.addUnit(std::move(ePtr));
    BattleState state{makeState(nullptr, &enemyParty)};

    ActionResult result{};
    result.actionAffinity = Affinity::Tempest;
    result.toughnessDamage = 0; // heal action — no toughness
    result.targetEnemyIndex = 0;

    auto hero = makeHero();
    vestige.onAction(*hero, result, state);

    CHECK(e->getToughness() == 50); // unchanged
}

// ── EchoingStrikeVestige ─────────────────────────────────────────────────────

TEST_CASE("EchoingStrikeVestige: flag is armed after ResonanceFieldTriggeredEvent")
{
    EchoingStrikeVestige vestige{};
    BattleState state{makeState()};

    vestige.onBattleStart(*reinterpret_cast<Battle *>(nullptr), state);
    CHECK(!vestige.isNextActionFree());

    state.eventBus.emit(ResonanceFieldTriggeredEvent{Affinity::Blaze, &state});
    CHECK(vestige.isNextActionFree());
}

TEST_CASE("EchoingStrikeVestige: refunds spCost to party and clears flag")
{
    EchoingStrikeVestige vestige{};
    Party playerParty;
    playerParty.gainSp(50);

    BattleState state{makeState(&playerParty, nullptr)};
    vestige.onBattleStart(*reinterpret_cast<Battle *>(nullptr), state);
    state.eventBus.emit(ResonanceFieldTriggeredEvent{Affinity::Frost, &state});
    REQUIRE(vestige.isNextActionFree());

    auto hero = makeHero();
    ActionResult result{};
    result.spCost = 20; // simulate a slot skill that cost 20 SP

    // Simulate the SP already having been spent by the action.
    playerParty.useSp(20);
    REQUIRE(playerParty.getSp() == 30);

    vestige.onAction(*hero, result, state);
    CHECK(playerParty.getSp() == 50);   // refunded
    CHECK(!vestige.isNextActionFree()); // flag consumed
}

TEST_CASE("EchoingStrikeVestige: flag not consumed by free actions (spCost == 0)")
{
    EchoingStrikeVestige vestige{};
    Party playerParty;
    playerParty.gainSp(30);
    BattleState state{makeState(&playerParty, nullptr)};

    vestige.onBattleStart(*reinterpret_cast<Battle *>(nullptr), state);
    state.eventBus.emit(ResonanceFieldTriggeredEvent{Affinity::Aether, &state});
    REQUIRE(vestige.isNextActionFree());

    auto hero = makeHero();
    ActionResult result{};
    result.spCost = 0; // Basic Attack — free

    vestige.onAction(*hero, result, state);
    CHECK(vestige.isNextActionFree()); // still armed
    CHECK(playerParty.getSp() == 30);  // unchanged
}

TEST_CASE("EchoingStrikeVestige: flag resets on new battle start")
{
    EchoingStrikeVestige vestige{};
    BattleState state{makeState()};

    // Arm the flag from a previous battle.
    vestige.onBattleStart(*reinterpret_cast<Battle *>(nullptr), state);
    state.eventBus.emit(ResonanceFieldTriggeredEvent{Affinity::Terra, &state});
    REQUIRE(vestige.isNextActionFree());

    // Simulate new battle start (new EventBus, reset flag).
    BattleState state2{makeState()};
    vestige.onBattleStart(*reinterpret_cast<Battle *>(nullptr), state2);
    CHECK(!vestige.isNextActionFree());
}

// ── VestigeOfTheUnbound ──────────────────────────────────────────────────────

TEST_CASE("VestigeOfTheUnbound: adds kExposurePerTurn to bearer each turn start")
{
    VestigeOfTheUnbound vestige{};
    auto hero = makeHero();
    BattleState state{makeState()};
    REQUIRE(hero->getExposure() == 0);

    vestige.onTurnStart(*hero, state);
    CHECK(hero->getExposure() == 5);

    vestige.onTurnStart(*hero, state);
    CHECK(hero->getExposure() == 10);
}

TEST_CASE("VestigeOfTheUnbound: exposure still accumulates at high values")
{
    VestigeOfTheUnbound vestige{};
    auto hero = makeHero();
    hero->modifyExposure(97); // close to cap
    BattleState state{makeState()};

    vestige.onTurnStart(*hero, state);
    CHECK(hero->getExposure() == 100); // clamped to max, not above
}

// ── AttunistGambitVestige ────────────────────────────────────────────────────

TEST_CASE("AttunistGambitVestige: reduces killer Exposure by kExposureOnKill on kill")
{
    AttunistGambitVestige vestige{};

    Party playerParty, enemyParty;
    auto heroRaw = makeHero();
    auto *hero = heroRaw.get();
    hero->modifyExposure(50); // 50 exposure before the kill
    playerParty.addUnit(std::move(heroRaw));

    BattleState state{makeState(&playerParty, &enemyParty)};
    vestige.onBattleStart(*reinterpret_cast<Battle *>(nullptr), state);

    // Emit kill event with hero as killer.
    auto enemy = makeEnemy();
    enemy->takeTrueDamage(9999); // kill it so isAlive() returns false
    state.eventBus.emit(UnitDefeatedEvent{enemy.get(), hero, &state});

    CHECK(hero->getExposure() == 42); // 50 - 8
}

TEST_CASE("AttunistGambitVestige: total reduction capped at kMaxReductionPerBattle")
{
    AttunistGambitVestige vestige{};

    Party playerParty, enemyParty;
    auto heroRaw = makeHero();
    auto *hero = heroRaw.get();
    hero->modifyExposure(100);
    hero->modifyExposure(-1); // 99 exposure (Vent sets to 0 so just force 99)
    hero->modifyExposure(0);
    // Set exposure to 80 to have room for reduction.
    hero->modifyExposure(-20); // now 79

    // Use modifyExposure to set a known value.
    hero->modifyExposure(-79);
    hero->modifyExposure(80);
    REQUIRE(hero->getExposure() == 80);

    playerParty.addUnit(std::move(heroRaw));
    BattleState state{makeState(&playerParty, &enemyParty)};
    vestige.onBattleStart(*reinterpret_cast<Battle *>(nullptr), state);

    auto e1 = makeEnemy();
    auto e2 = makeEnemy();
    auto e3 = makeEnemy();
    auto e4 = makeEnemy();
    auto e5 = makeEnemy();
    auto e6 = makeEnemy();

    // 6 kills × 8 = 48 potential reduction; capped at 40.
    state.eventBus.emit(UnitDefeatedEvent{e1.get(), hero, &state});
    state.eventBus.emit(UnitDefeatedEvent{e2.get(), hero, &state});
    state.eventBus.emit(UnitDefeatedEvent{e3.get(), hero, &state});
    state.eventBus.emit(UnitDefeatedEvent{e4.get(), hero, &state});
    state.eventBus.emit(UnitDefeatedEvent{e5.get(), hero, &state});
    state.eventBus.emit(UnitDefeatedEvent{e6.get(), hero, &state});

    // Max 40 reduction from 80 → 40 exposure.
    CHECK(hero->getExposure() == 40);
}

TEST_CASE("AttunistGambitVestige: amplifies positive exposureDelta by 1.5x")
{
    AttunistGambitVestige vestige{};
    auto hero = makeHero();
    BattleState state{makeState()};
    vestige.onBattleStart(*reinterpret_cast<Battle *>(nullptr), state);

    ActionResult result{};
    result.exposureDelta = 10;
    vestige.onAction(*hero, result, state);

    CHECK(result.exposureDelta == 15); // 10 * 1.5 = 15
}

TEST_CASE("AttunistGambitVestige: does not amplify negative exposureDelta (reductions)")
{
    AttunistGambitVestige vestige{};
    auto hero = makeHero();
    BattleState state{makeState()};
    vestige.onBattleStart(*reinterpret_cast<Battle *>(nullptr), state);

    ActionResult result{};
    result.exposureDelta = -30; // Purification Vial reducing exposure
    vestige.onAction(*hero, result, state);

    CHECK(result.exposureDelta == -30); // unchanged
}

TEST_CASE("AttunistGambitVestige: per-battle reduction counter resets on battle end")
{
    AttunistGambitVestige vestige{};

    Party playerParty, enemyParty;
    auto heroRaw = makeHero();
    auto *hero = heroRaw.get();
    hero->modifyExposure(80);
    playerParty.addUnit(std::move(heroRaw));

    BattleState state{makeState(&playerParty, &enemyParty)};
    vestige.onBattleStart(*reinterpret_cast<Battle *>(nullptr), state);

    // Exhaust the cap in battle 1.
    for (int i{0}; i < 5; ++i)
    {
        auto e = makeEnemy();
        state.eventBus.emit(UnitDefeatedEvent{e.get(), hero, &state});
    }
    REQUIRE(hero->getExposure() == 40); // reduced by 40 (cap)

    vestige.onBattleEnd(state);

    // Start battle 2 — reduction counter resets.
    BattleState state2{makeState(&playerParty, &enemyParty)};
    vestige.onBattleStart(*reinterpret_cast<Battle *>(nullptr), state2);
    hero->modifyExposure(40); // back to 80

    auto e = makeEnemy();
    state2.eventBus.emit(UnitDefeatedEvent{e.get(), hero, &state2});
    CHECK(hero->getExposure() == 72);
}

// ── Party vestige limit ───────────────────────────────────────────────────────

TEST_CASE("Party::addVestige: accepts vestiges up to kMaxVestiges")
{
    Party party;
    for (int i{0}; i < Party::kMaxVestiges; ++i)
    {
        bool ok{party.addVestige(std::make_unique<FlameResonanceVestige>())};
        CHECK(ok);
    }
    CHECK(party.getVestiges().size() == static_cast<std::size_t>(Party::kMaxVestiges));
}

TEST_CASE("Party::addVestige: returns false and does not add when at cap")
{
    Party party;
    for (int i{0}; i < Party::kMaxVestiges; ++i)
        party.addVestige(std::make_unique<FlameResonanceVestige>());

    bool ok{party.addVestige(std::make_unique<VestigeOfTheUnbound>())};
    CHECK(!ok);
    CHECK(party.getVestiges().size() == static_cast<std::size_t>(Party::kMaxVestiges));
}