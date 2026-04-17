#pragma once
/**
 * @file BurnEffect.h
 * @brief Burn DoT: deals fixed true damage per tick, bypassing shields.
 */

#include "Core/StatusEffectBase.h"

/** Debuff | DoT | Blaze. Deals m_damagePerTick true damage each tick. */
class BurnEffect : public StatusEffectBase
{
  public:
    /**
     * @param damagePerTick HP damage per tick (bypasses shield absorption).
     * @param duration      Ticks before natural expiry.
     */
    explicit BurnEffect(int damagePerTick = 5, int duration = 2);

    bool isBuff() const override
    {
        return false;
    }
    bool isDebuff() const override
    {
        return true;
    }

    /**
     * @brief Deals m_damagePerTick true damage to owner.
     * @return Message describing the damage dealt.
     */
    std::string onTick(Unit &owner) override;

  private:
    int m_damagePerTick;
};