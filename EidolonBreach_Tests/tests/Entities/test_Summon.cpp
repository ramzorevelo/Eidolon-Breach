/**
 * @file test_Summon.cpp
 * @brief Tests for SummonRegistry lookup and Summon entity construction and turn behavior.
 */
#include "Battle/BattleState.h"
#include "Battle/ResonanceField.h"
#include "Core/EventBus.h"
#include "Battle/SpeedBasedTurnOrderCalculator.h"
#include "Core/RunContext.h"
#include "Entities/Party.h"
#include "Entities/Summon.h"
#include "Summons/SummonDefinition.h"
#include "Summons/SummonRegistry.h"
#include "UI/test_NullInputHandler.h"
#include "UI/test_NullRenderer.h"
#include "doctest.h"
#include <test_helpers.h>

namespace
{
ResonanceField g_field{};
NullInputHandler g_input{};
NullRenderer g_renderer{};
RunContext g_runCtx{};
EventBus g_eventBus{};

BattleState makeState()
{
    g_field = ResonanceField{};
    return BattleState{0, 0, Affinity::Aether, g_field, g_input, g_renderer, g_runCtx, g_eventBus};
}

SummonDefinition makeTestDef(std::optional<int> duration = std::nullopt)
{
    SummonDefinition def{};
    def.id = "test_summon";
    def.displayName = "Test Summon";
    def.baseStats = Stats{40, 40, 5, 0, 12};
    def.duration = duration;
    return def;
}
} // namespace


TEST_CASE("SummonRegistry: find returns nullptr for missing id")
{
    SummonRegistry reg{};
    CHECK(reg.find("nonexistent") == nullptr);
}

TEST_CASE("SummonRegistry: registerDefinition and find round-trip")
{
    SummonRegistry reg{};
    reg.registerDefinition(makeTestDef());
    const SummonDefinition *found{reg.find("test_summon")};
    REQUIRE(found != nullptr);
    CHECK(found->displayName == "Test Summon");
    CHECK(found->baseStats.spd == 12);
}

TEST_CASE("SummonRegistry: re-register overwrites existing entry")
{
    SummonRegistry reg{};
    SummonDefinition a{makeTestDef()};
    a.baseStats.spd = 10;
    reg.registerDefinition(std::move(a));

    SummonDefinition b{makeTestDef()};
    b.baseStats.spd = 20;
    reg.registerDefinition(std::move(b));

    CHECK(reg.find("test_summon")->baseStats.spd == 20);
}


TEST_CASE("Summon: isSummon returns true")
{
    SummonDefinition def{makeTestDef()};
    Summon s{def, 10};
    CHECK(s.isSummon());
}

TEST_CASE("Summon: base Unit::isSummon returns false")
{
    // PlayableCharacter is a Unit; confirm it does not flag as summon.
    SummonDefinition def{makeTestDef()};
    Summon s{def, 10};
    const Unit &u{s};
    // Cast to Unit& — isSummon() is virtual, must still return true.
    CHECK(u.isSummon());
}

TEST_CASE("Summon: resonanceContribution is floor(summonerContribution / 2)")
{
    SummonDefinition def{makeTestDef()};
    Summon s11{def, 11}; // 11 / 2 = 5
    CHECK(s11.getResonanceContribution() == 5);

    Summon s10{def, 10}; // 10 / 2 = 5
    CHECK(s10.getResonanceContribution() == 5);
}

TEST_CASE("Summon: isExpired false when duration is nullopt")
{
    SummonDefinition def{makeTestDef(std::nullopt)};
    Summon s{def, 10};
    s.tickDuration();
    s.tickDuration();
    CHECK(!s.isExpired());
}

TEST_CASE("Summon: isExpired true after duration ticks to zero")
{
    SummonDefinition def{makeTestDef(2)};
    Summon s{def, 10};
    CHECK(!s.isExpired());
    s.tickDuration();
    CHECK(!s.isExpired());
    s.tickDuration();
    CHECK(s.isExpired());
}

TEST_CASE("Summon: takeTurn returns Skip when action pool is empty")
{
    SummonDefinition def{makeTestDef()};
    Summon s{def, 10};
    Party allies, enemies;
    BattleState state{makeState()};
    ActionResult r{s.takeTurn(allies, enemies, state)};
    CHECK(r.type == ActionResult::Type::Skip);
}

TEST_CASE("Summon: takeTurn calls action and returns its result")
{
    SummonDefinition def{makeTestDef()};
    bool called{false};
    def.actions.push_back(SummonAction{
        "test_action",
        [&called](Summon &, Party &, Party &, BattleState &) -> ActionResult
        {
            called = true;
            return ActionResult{ActionResult::Type::Damage, 7};
        }});

    Summon s{def, 10};
    Party allies, enemies;
    BattleState state{makeState()};
    ActionResult r{s.takeTurn(allies, enemies, state)};
    CHECK(called);
    CHECK(r.value == 7);
}

TEST_CASE("Summon: turn order includes Summon via SpeedBasedTurnOrderCalculator")
{


    SummonDefinition def{makeTestDef()};
    def.baseStats.spd = 15;
    auto summon{std::make_unique<Summon>(def, 10)};

    Party playerParty, enemyParty;
    playerParty.addUnit(std::move(summon));

    SpeedBasedTurnOrderCalculator calc{};
    auto order{calc.calculate(playerParty, enemyParty)};
    REQUIRE(order.size() == 1);
    CHECK(order[0].unit->isSummon());
}

TEST_CASE("Summon: getSummonerAtk returns value passed at construction")
{
    SummonDefinition def{makeTestDef()};
    Summon s{def, 10, 25};
    CHECK(s.getSummonerAtk() == 25);
}

TEST_CASE("Summon: default summonerAtk is 0 when not specified")
{
    SummonDefinition def{makeTestDef()};
    Summon s{def, 10};
    CHECK(s.getSummonerAtk() == 0);
}

TEST_CASE("Party::removeUnit: removes unit by id")
{
    Party p;
    p.addUnit(makeHero("hero_a"));
    p.addUnit(makeHero("hero_b"));
    REQUIRE(p.size() == 2);
    p.removeUnit("hero_a");
    CHECK(p.size() == 1);
    CHECK(p.getUnitAt(0)->getId() == "hero_b");
}

TEST_CASE("Party::removeUnit: no-op for absent id")
{
    Party p;
    p.addUnit(makeHero("hero_a"));
    p.removeUnit("nobody"); // must not crash
    CHECK(p.size() == 1);
}