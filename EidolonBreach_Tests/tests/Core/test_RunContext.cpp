/**
 * @file test_RunContext.cpp
 * @brief Tests for RunContext signal tracking, crystallization state, and reset.
 */
#include "Core/BehaviorSignal.h"
#include "Core/RunContext.h"
#include "doctest.h"
#include <string_view>

TEST_CASE("RunContext: getCharacterState creates default entry on first access")
{
    RunContext ctx{};
    const RunCharacterState &cs{ctx.getCharacterState("hero_1")};
    CHECK(cs.synchronicityProgress == 0);
    CHECK(!cs.crystallizedStanceId.has_value());
    CHECK(cs.signalCounts.empty());
}

TEST_CASE("RunContext: getCharacterState returns same entry on repeated access")
{
    RunContext ctx{};
    RunCharacterState &a{ctx.getCharacterState("hero_1")};
    a.synchronicityProgress = 42;
    RunCharacterState &b{ctx.getCharacterState("hero_1")};
    CHECK(b.synchronicityProgress == 42);
}

TEST_CASE("RunContext: string_view key lookup works without std::string construction")
{
    RunContext ctx{};
    ctx.getCharacterState("hero_1").synchronicityProgress = 10;
    std::string_view sv{"hero_1"};
    const RunCharacterState *found{ctx.findCharacterState(sv)};
    REQUIRE(found != nullptr);
    CHECK(found->synchronicityProgress == 10);
}

TEST_CASE("RunContext: findCharacterState returns nullptr for unknown id")
{
    RunContext ctx{};
    CHECK(ctx.findCharacterState("nobody") == nullptr);
}

TEST_CASE("RunContext: signal counts increment independently per signal")
{
    RunContext ctx{};
    RunCharacterState &cs{ctx.getCharacterState("hero")};
    ++cs.signalCounts[BehaviorSignal::Aggressive];
    ++cs.signalCounts[BehaviorSignal::Aggressive];
    ++cs.signalCounts[BehaviorSignal::Methodical];
    CHECK(cs.signalCounts[BehaviorSignal::Aggressive] == 2);
    CHECK(cs.signalCounts[BehaviorSignal::Methodical] == 1);
}

TEST_CASE("RunContext: crystallizedStanceId starts as nullopt")
{
    RunContext ctx{};
    CHECK(!ctx.getCharacterState("h").crystallizedStanceId.has_value());
}

TEST_CASE("RunContext: reset clears all entries and vote totals")
{
    RunContext ctx{};
    ctx.getCharacterState("h1").synchronicityProgress = 50;
    ctx.recordFieldVotes(Affinity::Blaze, 3.0f);
    ctx.reset();
    CHECK(ctx.findCharacterState("h1") == nullptr);
    CHECK(ctx.getAffinityVoteTotal(Affinity::Blaze) == doctest::Approx(0.0f));
}

TEST_CASE("RunContext: recordFieldVotes accumulates across calls")
{
    RunContext ctx{};
    ctx.recordFieldVotes(Affinity::Frost, 2.5f);
    ctx.recordFieldVotes(Affinity::Frost, 1.5f);
    CHECK(ctx.getAffinityVoteTotal(Affinity::Frost) == doctest::Approx(4.0f));
}

TEST_CASE("RunContext: getAffinityVoteTotal returns 0 for affinity with no votes")
{
    RunContext ctx{};
    CHECK(ctx.getAffinityVoteTotal(Affinity::Terra) == doctest::Approx(0.0f));
}

TEST_CASE("RunContext: runMode defaults to Classic")
{
    RunContext ctx{};
    CHECK(ctx.runMode == RunMode::Classic);
}

TEST_CASE("RunContext: runMode can be set to Draft")
{
    RunContext ctx{};
    ctx.runMode = RunMode::EidolonLabyrinth;
    CHECK(ctx.runMode == RunMode::EidolonLabyrinth);
}

TEST_CASE("RunContext: reset preserves runMode (mode set by Dungeon, not per-battle)")
{
    RunContext ctx{};
    ctx.runMode = RunMode::EidolonLabyrinth;
    ctx.reset(); // resets signal counts and vote totals, not mode
    CHECK(ctx.runMode == RunMode::EidolonLabyrinth);
}

TEST_CASE("RunContext: Training mode can be set")
{
    RunContext ctx{};
    ctx.runMode = RunMode::Training;
    CHECK(ctx.runMode == RunMode::Training);
}