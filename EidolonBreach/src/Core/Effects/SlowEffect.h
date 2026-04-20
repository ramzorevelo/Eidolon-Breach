#pragma once
/**
 * @file SlowEffect.h
 * @brief Slow debuff: reduces SPD by percentage in the percentage modifier pass.
 */

#include "Core/StatusEffectBase.h"

/** Debuff | StatMod | Frost. Reduces SPD by m_slowRatio in the percentage modifier pass. */
class SlowEffect : public StatusEffectBase
{
  public:
    /**
     * @param slowRatio  Fraction of SPD to subtract (e.g. 0.30f = 30% reduction).
     * @param duration   Ticks before natural expiry.
     */
    explicit SlowEffect(float slowRatio = 0.30f, int duration = 2);

    bool isBuff() const override
    {
        return false;
    }
    bool isDebuff() const override
    {
        return true;
    }

    /**
     * @brief Reduces base.spd by m_slowRatio in pass 2 (percentage).
     *
     * Applied after all flat additions so that the slowRatio operates on
     * the already-flat-modified SPD value (see §2.12.3 two-pass order).
     */
    Stats modifyStatsPct(Stats base) const override;

  private:
    float m_slowRatio;
};