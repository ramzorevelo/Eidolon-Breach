#pragma once
/**
 * @file CharacterMod.h
 * @brief Permanent per-character modifiers sourced from Aspect Tree nodes and
 *        Resonance Echo effects. Applied by CharacterRegistry::create().
 *
 * CharacterMod is a std::variant — add new members via std::visit without
 * touching PlayableCharacter for each new node type.
 */
#include <variant>

/**
 * @brief Identifies which Exposure state an AVModifierBonus targets.
 *        Matches the five states in PlayableCharacter::getExposureAVModifier().
 */
enum class ExposureState
{
    Baseline,
    Resonating,
    Surging,
    Breachborn,
    Fractured,
};

/** Flat stat increase. hp maps to maxHp (current HP is reset to new max). */
struct StatBonus
{
    int hp{0};
    int atk{0};
    int def{0};
    int spd{0};
};

/** Reduces the arch skill cooldown by this many turns (minimum effective: 0). */
struct CooldownReduction
{
    int turns{0};
};

/**
 * @brief Shifts the Exposure level at which Resonating or Surging activates.
 *        A negative delta means the state activates sooner (lower Exposure).
 *        threshold must be 50 (Resonating) or 75 (Surging).
 */
struct ExposureThresholdShift
{
    int threshold{50};
    int delta{0};
};

/**
 * @brief Causes slot `slot` to unlock at `levelRequired` instead of the
 *        standard constant. Only takes effect if levelRequired < standard level.
 */
struct SlotUnlockEarly
{
    int slot{0};
    int levelRequired{0};
};

/**
 * @brief Scales all Resonating, Surging, and Breachborn proc damage and
 *        duration for this character by `multiplier`.
 */
struct ProcEnhancement
{
    float multiplier{1.0f};
};

/**
 * @brief Adds `delta` to the raw AV modifier constant for `state`.
 *        Negative delta acts sooner (speed bonus); positive acts later (penalty).
 *        Applied additively on top of the CombatConstants baseline.
 */
struct AVModifierBonus
{
    ExposureState state{ExposureState::Baseline};
    float delta{0.0f};
};

using CharacterMod = std::variant<
    StatBonus,
    CooldownReduction,
    ExposureThresholdShift,
    SlotUnlockEarly,
    ProcEnhancement,
    AVModifierBonus>;