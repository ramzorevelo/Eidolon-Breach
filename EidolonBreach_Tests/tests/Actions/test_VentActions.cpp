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

TEST_CASE("VentAction: isAvailable returns false until Exposure is wired (Commit 10)")
{
    Party allies{};
    auto hero = makeHero();
    VentAction vent{};
    CHECK(!vent.isAvailable(*hero, allies));
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