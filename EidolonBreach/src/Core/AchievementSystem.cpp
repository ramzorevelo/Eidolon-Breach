/**
 * @file AchievementSystem.cpp
 * @brief AchievementSystem stub implementation.
 */

#include "Core/AchievementSystem.h"
#include "Core/BattleEvents.h"

AchievementSystem::AchievementSystem(EventBus &eventBus)
{
    // Subscriptions are Run-scoped: they survive individual battles but are
    // cleared when Dungeon::run() calls eventBus.clearRunScope().
    eventBus.subscribe<UnitDefeatedEvent>(
        [](const UnitDefeatedEvent &) { /* Phase 8: check kill-count achievements */ },
        EventScope::Run);

    eventBus.subscribe<BreakTriggeredEvent>(
        [](const BreakTriggeredEvent &) { /* Phase 8: check break-count achievements */ },
        EventScope::Run);

    eventBus.subscribe<RunCompletedEvent>(
        [](const RunCompletedEvent &) { /* Phase 8: check run-completion achievements */ },
        EventScope::Run);
}