#pragma once
/**
 * @file StatusEffectBase.h
 * @brief Concrete base providing duration management and tag boilerplate.
 *
 * Concrete effects (BurnEffect, SlowEffect, etc.) inherit from this class
 * and override only the hooks they need. Duration decrement is handled here
 * via extendDuration(-1) called from Unit::tickEffects() — concrete effects
 * must never touch m_duration directly.
 */

#include "Core/IStatusEffect.h"
#include <initializer_list>

class StatusEffectBase : public IStatusEffect
{
  public:
    /**
     * @param id        Effect ID string (use EffectIds constants, not raw literals).
     * @param name      Display name.
     * @param duration  Remaining turns, or std::nullopt for permanent effects.
     * @param tags      Category tags (use EffectTags constants).
     */
    StatusEffectBase(std::string_view id,
                     std::string_view name,
                     std::optional<int> duration,
                     std::initializer_list<std::string_view> tags);

    // --- IStatusEffect: Identity ---
    std::string_view getId() const override;
    std::string_view getName() const override;

    // --- IStatusEffect: Duration ---
    std::optional<int> getDuration() const override;

    // --- IStatusEffect: Tags ---
    const std::vector<std::string> &getTags() const override;
    bool hasTag(std::string_view tag) const override;

    // --- IStatusEffect: No-op defaults (override in subclasses as needed) ---
    void onApply(Unit & /*owner*/) override {}
    std::string onTick(Unit & /*owner*/) override
    {
        return {};
    }
    void onExpire(Unit & /*owner*/) override {}
    void onRemove(Unit & /*owner*/) override {}

    Stats modifyStatsFlat(Stats base) const override
    {
        return base;
    }
    Stats modifyStatsPct(Stats base) const override
    {
        return base;
    }

    int absorbDamage(int incoming) override
    {
        return incoming;
    }
    bool isExhausted() const override
    {
        return false;
    }

    void extendDuration(int turns) override;

  protected:
    std::string m_id;
    std::string m_name;
    std::optional<int> m_duration;
    std::vector<std::string> m_tags;
};