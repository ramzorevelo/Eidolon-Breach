#pragma once
/**
 * @file BattleEvents.h
 * @brief All concrete event types emitted during battle and run lifecycle.
 *        Plain data structs — no virtual methods, no inheritance.
 *        Add new events here; never emit events with zero subscribers.
 */

#include "Core/Affinity.h"
#include <string>

class Unit;
class PlayableCharacter;
class Enemy;
struct BattleState;

/** Emitted when any unit's HP reaches 0. killer is nullptr for DoT or environmental kills. */
struct UnitDefeatedEvent
{
    Unit *defeated{nullptr};
    Unit *killer{nullptr};
    BattleState *state{nullptr};
};

/** Emitted when an enemy's Toughness reaches 0 and the break fires. */
struct BreakTriggeredEvent
{
    Unit *broken{nullptr};
    Affinity affinity{};
    BattleState *state{nullptr};
};

/** Emitted when the Resonance Field gauge hits 100 and the trigger fires. */
struct ResonanceFieldTriggeredEvent
{
    Affinity triggeredAffinity{};
    BattleState *state{nullptr};
};

/** Emitted when a character's Exposure first crosses 50, 75, or reaches 100. */
struct ExposureThresholdEvent
{
    PlayableCharacter *character{nullptr};
    int threshold{};
    BattleState *state{nullptr};
};

/** Emitted when a character's dominant behavioral signal crystallizes into a Stance. */
struct StanceCrystallizedEvent
{
    PlayableCharacter *character{nullptr};
    std::string stanceId{};
};

/** Emitted at the start of Battle::run(), before the first turn. */
struct BattleStartedEvent
{
    BattleState *state{nullptr};
};

/** Emitted when Battle::run() resolves, before cleanup. */
struct BattleEndedEvent
{
    bool playerWon{false};
    BattleState *state{nullptr};
};

/** Emitted by AchievementSystem when a milestone is met. UI subscribes. */
struct AchievementUnlockedEvent
{
    std::string achievementId{};
};

/** Emitted at the very end of a run (success or failure). */
struct RunCompletedEvent
{
    bool success{false};
    int floorsCleared{0};
};