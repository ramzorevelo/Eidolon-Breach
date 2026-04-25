#pragma once
/**
 * @file AchievementSystem.h
 * @brief Subscribes to battle and run events to track achievement progress.
 *        Phase 7 stub: handlers are registered but contain no logic.
 *        Full threshold checking and AchievementUnlockedEvent emission in Phase 8.
 */

#include "Core/EventBus.h"

class AchievementSystem
{
  public:
    /**
     * @brief Registers run-scoped subscriptions on the provided EventBus.
     *        Subscriptions survive individual battles but are cleared at run end.
     * @param eventBus The dungeon-owned EventBus.
     */
    explicit AchievementSystem(EventBus &eventBus);
};