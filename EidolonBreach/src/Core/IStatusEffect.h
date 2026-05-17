#pragma once
/**
 * @file IStatusEffect.h
 * @brief Interface for all temporary and permanent Unit modifiers.
 */

#include "Core/Stats.h"
#include <optional>
#include <string>
#include <string_view>
#include <vector>

class Unit;

/**
 * @brief Governs every buff, debuff, DoT, shield, and HoT applied to a Unit.
 *
 * Concrete effects inherit from StatusEffectBase, which provides duration
 * management, tag storage, and no-op defaults. Call sites interact via this
 * interface only.
 */
class IStatusEffect
{
  public:
    virtual ~IStatusEffect() = default;

    // Identity
    /** @return Unique lowercase identifier (e.g. "burn"). Use EffectIds constants. */
    virtual std::string_view getId() const = 0;
    /** @return Display name (e.g. "Burn"). */
    virtual std::string_view getName() const = 0;

    // Duration
    /**
     * @brief Duration in ticks remaining, or std::nullopt for permanent effects.
     * Permanent effects are never removed by the duration check in tickEffects().
     */
    virtual std::optional<int> getDuration() const = 0;

    // Categorisation
    virtual bool isBuff() const = 0;
    virtual bool isDebuff() const = 0;

    // Tag System
    /** @return All tags on this effect (e.g. {"Debuff", "DoT", "Blaze"}). */
    virtual const std::vector<std::string> &getTags() const = 0;
    /** @return true if this effect carries the given tag. */
    virtual bool hasTag(std::string_view tag) const = 0;

    // Lifecycle Hooks
    /** Called once when the effect is first applied to a Unit. */
    virtual void onApply(Unit &owner) = 0;
    /**
     * @brief Called each tick (once per unit turn).
     * @return Human-readable message for the UI, or empty string if silent.
     */
    virtual std::string onTick(Unit &owner) = 0;
    /** Called when duration reaches 0 (natural expiry). */
    virtual void onExpire(Unit &owner) = 0;
    /** Called when the effect is removed before natural expiry (e.g. cleanse). */
    virtual void onRemove(Unit &owner) = 0;

    // Stat Modification
    /** Pass 1: flat additions. Return modified Stats. */
    virtual Stats modifyStatsFlat(Stats base) const = 0;
    /** Pass 2: percentage multipliers applied after all flat mods. Return modified Stats. */
    virtual Stats modifyStatsPct(Stats base) const = 0;

    // Shield Absorption
    /**
     * @brief Attempt to absorb incoming direct damage.
     * @param incoming Damage amount after DEF reduction.
     * @return Overflow damage that was not absorbed (hits HP directly).
     *
     * Non-shield effects return `incoming` unchanged (absorb nothing).
     * DoT ticks call takeTrueDamage() and bypass this entirely.
     */
    virtual int absorbDamage(int incoming) = 0;

    /**
     * @brief Return the current remaining shield capacity.
     *        Default 0; ShieldEffect overrides.
     */
    [[nodiscard]] virtual int getShieldAmount() const
    {
        return 0;
    }

    /** @return true when a shield's absorb pool is fully depleted. */
    virtual bool isExhausted() const = 0;

    // Duration Extension
    /**
     * @brief Add turns to the remaining duration.
     * Passing a negative value decrements. Has no effect on permanent effects.
     */
    virtual void extendDuration(int turns) = 0;
};