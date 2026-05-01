/**
 * @file test_FieldDiscovery.cpp
 * @brief Tests for RunContext::checkAndActivateDiscoveries thresholds.
 */
#include "Core/Affinity.h"
#include "Core/FieldDiscovery.h"
#include "Core/RunContext.h"
#include "doctest.h"

TEST_CASE("FieldDiscovery: Molten Lattice activates on Blaze >= 60% and Terra >= 30%")
{
    RunContext ctx{};
    // Simulate 70 Blaze, 30 Terra votes (total 100)
    ctx.recordFieldVotes(Affinity::Blaze, 70.0f);
    ctx.recordFieldVotes(Affinity::Terra, 30.0f);
    ctx.checkAndActivateDiscoveries();
    CHECK(ctx.activeDiscoveries.count(std::string{FieldDiscoveryIds::kMoltenLattice}) == 1);
}

TEST_CASE("FieldDiscovery: Molten Lattice does not activate when Terra is below threshold")
{
    RunContext ctx{};
    ctx.recordFieldVotes(Affinity::Blaze, 80.0f);
    ctx.recordFieldVotes(Affinity::Terra, 10.0f);
    ctx.checkAndActivateDiscoveries();
    CHECK(ctx.activeDiscoveries.count(std::string{FieldDiscoveryIds::kMoltenLattice}) == 0);
}

TEST_CASE("FieldDiscovery: Arctic Surge activates on Frost >= 60% and Tempest >= 30%")
{
    RunContext ctx{};
    ctx.recordFieldVotes(Affinity::Frost, 62.0f);
    ctx.recordFieldVotes(Affinity::Tempest, 38.0f);
    ctx.checkAndActivateDiscoveries();
    CHECK(ctx.activeDiscoveries.count(std::string{FieldDiscoveryIds::kArcticSurge}) == 1);
}

TEST_CASE("FieldDiscovery: Lattice Attunement activates on Aether >= 30% and one affinity >= 50%")
{
    RunContext ctx{};
    ctx.recordFieldVotes(Affinity::Aether, 30.0f);
    ctx.recordFieldVotes(Affinity::Blaze, 55.0f);
    ctx.recordFieldVotes(Affinity::Frost, 15.0f);
    ctx.checkAndActivateDiscoveries();
    CHECK(ctx.activeDiscoveries.count(std::string{FieldDiscoveryIds::kLatticeAttunement}) == 1);
}

TEST_CASE("FieldDiscovery: Lattice Attunement does not activate when no affinity dominates")
{
    RunContext ctx{};
    ctx.recordFieldVotes(Affinity::Aether, 30.0f);
    ctx.recordFieldVotes(Affinity::Blaze, 35.0f);
    ctx.recordFieldVotes(Affinity::Frost, 35.0f);
    ctx.checkAndActivateDiscoveries();
    CHECK(ctx.activeDiscoveries.count(std::string{FieldDiscoveryIds::kLatticeAttunement}) == 0);
}

TEST_CASE("FieldDiscovery: no activation when total votes is zero")
{
    RunContext ctx{};
    ctx.checkAndActivateDiscoveries(); // must not divide by zero
    CHECK(ctx.activeDiscoveries.empty());
}

TEST_CASE("FieldDiscovery: discoveries persist across multiple checks")
{
    RunContext ctx{};
    ctx.recordFieldVotes(Affinity::Blaze, 70.0f);
    ctx.recordFieldVotes(Affinity::Terra, 30.0f);
    ctx.checkAndActivateDiscoveries();
    ctx.checkAndActivateDiscoveries(); // second call must not duplicate or remove
    CHECK(ctx.activeDiscoveries.count(std::string{FieldDiscoveryIds::kMoltenLattice}) == 1);
}

TEST_CASE("RunContext::reset clears active discoveries")
{
    RunContext ctx{};
    ctx.recordFieldVotes(Affinity::Blaze, 70.0f);
    ctx.recordFieldVotes(Affinity::Terra, 30.0f);
    ctx.checkAndActivateDiscoveries();
    REQUIRE(!ctx.activeDiscoveries.empty());
    ctx.reset();
    CHECK(ctx.activeDiscoveries.empty());
} 