/**
 * @file test_StanceModifiers.cpp
 * @brief Tests for StanceModifiers::resolveStanceId and applyResonanceModifier.
 */
#include "Battle/BattleState.h"
#include "Battle/ResonanceField.h"
#include "Battle/StanceModifiers.h"
#include "Characters/Lyra.h"
#include "Core/BehaviorSignal.h"
#include "Core/EventBus.h"
#include "Core/RunContext.h"
#include "UI/test_NullInputHandler.h"
#include "UI/test_NullRenderer.h"
#include "doctest.h"
#include "test_helpers.h"

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
} // namespace


TEST_CASE("resolveStanceId: Lyra Aggressive → Predator")
{
    CHECK(StanceModifiers::resolveStanceId(LyraIds::kId,
                                           BehaviorSignal::Aggressive) == LyraStances::kPredator);
}

TEST_CASE("resolveStanceId: Lyra Methodical → Conflagration")
{
    CHECK(StanceModifiers::resolveStanceId(LyraIds::kId,
                                           BehaviorSignal::Methodical) == LyraStances::kConflagration);
}

TEST_CASE("resolveStanceId: Lyra Sacrificial → Ember")
{
    CHECK(StanceModifiers::resolveStanceId(LyraIds::kId,
                                           BehaviorSignal::Sacrificial) == LyraStances::kEmber);
}

TEST_CASE("resolveStanceId: unrecognised character returns empty string_view")
{
    CHECK(StanceModifiers::resolveStanceId("unknown_hero",
                                           BehaviorSignal::Aggressive)
              .empty());
}


TEST_CASE("applyResonanceModifier: empty stanceId returns baseAmount unchanged")
{
    auto hero = makeHero();
    BattleState state{makeState()};
    int result{StanceModifiers::applyResonanceModifier(
        "", *hero, Affinity::Blaze, 10, state)};
    CHECK(result == 10);
}

TEST_CASE("applyResonanceModifier: Predator adds +1 to Blaze actions")
{
    auto hero = makeHero();
    BattleState state{makeState()};
    int result{StanceModifiers::applyResonanceModifier(
        LyraStances::kPredator, *hero, Affinity::Blaze, 10, state)};
    CHECK(result == 11);
}

TEST_CASE("applyResonanceModifier: Predator does not modify non-Blaze actions")
{
    auto hero = makeHero();
    BattleState state{makeState()};
    int result{StanceModifiers::applyResonanceModifier(
        LyraStances::kPredator, *hero, Affinity::Frost, 10, state)};
    CHECK(result == 10);
}

TEST_CASE("applyResonanceModifier: Conflagration adds +8 to Blaze actions")
{
    auto hero = makeHero();
    BattleState state{makeState()};
    int result{StanceModifiers::applyResonanceModifier(
        LyraStances::kConflagration, *hero, Affinity::Blaze, 10, state)};
    CHECK(result == 18);
}

TEST_CASE("applyResonanceModifier: Ember adds +2 to any affinity")
{
    auto hero = makeHero();
    BattleState state{makeState()};
    int result{StanceModifiers::applyResonanceModifier(
        LyraStances::kEmber, *hero, Affinity::Terra, 10, state)};
    CHECK(result == 12);
}

TEST_CASE("applyResonanceModifier: unrecognised stanceId returns baseAmount")
{
    auto hero = makeHero();
    BattleState state{makeState()};
    int result{StanceModifiers::applyResonanceModifier(
        "invalid_stance", *hero, Affinity::Blaze, 10, state)};
    CHECK(result == 10);
}