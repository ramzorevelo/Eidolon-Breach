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
        [](const UnitDefeatedEvent &) {},
        EventScope::Run);

    eventBus.subscribe<BreakTriggeredEvent>(
        [](const BreakTriggeredEvent &) {},
        EventScope::Run);

    eventBus.subscribe<RunCompletedEvent>(
        [](const RunCompletedEvent &) {},
        EventScope::Run);
}