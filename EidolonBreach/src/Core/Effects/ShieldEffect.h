#pragma once
/**
 * @file ShieldEffect.h
 * @brief Shield buff: absorbs direct damage before HP is reduced.
 *
 * DoT ticks call takeTrueDamage() and bypass this effect entirely.
 * The shield is exhausted when m_remainingAbsorb reaches 0.
 */

#include "Core/StatusEffectBase.h"

/** Buff | Shield | Terra. Absorbs up to m_remainingAbsorb direct damage. */
class ShieldEffect : public StatusEffectBase
{
  public:
    /**
     * @param absorbAmount Maximum HP damage this shield absorbs.
     * @param duration     Ticks before expiry even if not depleted.
     */
    explicit ShieldEffect(int absorbAmount, int duration = 3);

    bool isBuff() const override
    {
        return true;
    }
    bool isDebuff() const override
    {
        return false;
    }

    /**
     * @brief Absorbs up to m_remainingAbsorb of incoming direct damage.
     * @param incoming Damage after DEF reduction.
     * @return Overflow that passes through to HP.
     */
    int absorbDamage(int incoming) override;

    /** @return true when the absorb pool is fully depleted. */
    bool isExhausted() const override;

  private:
    int m_remainingAbsorb;
};