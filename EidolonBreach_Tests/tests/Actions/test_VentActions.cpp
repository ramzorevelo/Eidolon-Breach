/**
 * @file test_VentAction.cpp
 * @brief Tests for VentAction stub behavior (pre-Exposure wiring).
 */
#include "Actions/VentAction.h"
#include "Entities/Party.h"
#include "doctest.h"
#include "test_helpers.h"
#include <ostream>
#include <string>

TEST_CASE("VentAction: label is correct")
{
    VentAction vent{};
    CHECK(vent.label() == "Vent (Exposure -> 0, ends turn)");
}

TEST_CASE("VentAction: isAvailable false when Fractured even with valid Exposure")
{
    Party allies{};
    auto hero = makeHero();
    hero->modifyExposure(50);
    REQUIRE(hero->canVent());

    hero->applyFracture();
    VentAction vent{};
    CHECK(!vent.isAvailable(*hero, allies));
}

TEST_CASE("VentAction: execute sets ventConsolation true when Exposure >= 50")
{
    Party allies{}, enemies{};
    auto hero = makeHero();
    hero->modifyExposure(75);
    VentAction vent{};
    ActionResult result{vent.execute(*hero, allies, enemies, std::nullopt)};
    CHECK(result.ventConsolation);
    CHECK(hero->getExposure() == 0);
}

TEST_CASE("VentAction: execute sets ventConsolation false when Exposure < 50")
{
    Party allies{}, enemies{};
    auto hero = makeHero();
    hero->modifyExposure(30);
    VentAction vent{};
    ActionResult result{vent.execute(*hero, allies, enemies, std::nullopt)};
    CHECK(!result.ventConsolation);
}

TEST_CASE("VentAction: execute cancels armed Surging proc")
{
    Party allies{}, enemies{};
    auto hero = makeHero();
    hero->modifyExposure(75);
    hero->armSurgingProc();
    REQUIRE(hero->isSurgingProcArmed());

    VentAction vent{};
    vent.execute(*hero, allies, enemies, std::nullopt);
    CHECK(!hero->isSurgingProcArmed()); // cancelled on vent
}

TEST_CASE("VentAction: execute does not cancel Surging proc when Exposure < 50")
{
    Party allies{}, enemies{};
    auto hero = makeHero();
    hero->modifyExposure(30);
    hero->armSurgingProc();

    VentAction vent{};
    vent.execute(*hero, allies, enemies, std::nullopt);
    CHECK(hero->isSurgingProcArmed()); // not cancelled — below consolation threshold
}

TEST_CASE("VentAction: execute returns Skip result")
{
    Party allies{}, enemies{};
    auto hero = makeHero();
    VentAction vent{};
    ActionResult result = vent.execute(*hero, allies, enemies, std::nullopt);
    CHECK(result.type == ActionResult::Type::Skip);
}

TEST_CASE("VentAction: affinity is Aether")
{
    VentAction vent{};
    CHECK(vent.getAffinity() == Affinity::Aether);
}

TEST_CASE("VentAction: ActionData has zero costs and Self targeting")
{
    VentAction vent{};
    const ActionData &data = vent.getActionData();
    CHECK(data.spCost == 0);
    CHECK(data.momentumCost == 0);
    CHECK(data.momentumGain == 0);
    CHECK(data.targetMode == TargetMode::Self);
}