#pragma once
/**
 * @file RegenEffect.h
 * @brief Regen HoT: restores HP per tick up to the unit's max HP.
 */

#include "Core/StatusEffectBase.h"

/** Buff | HoT | Terra. Heals m_healPerTick HP each tick (capped at maxHp). */
class RegenEffect : public StatusEffectBase
{
  public:
    /**
     * @param healPerTick HP restored each tick.
     * @param duration    Ticks before natural expiry.
     */
    explicit RegenEffect(int healPerTick = 10, int duration = 2);

    bool isBuff() const override
    {
        return true;
    }
    bool isDebuff() const override
    {
        return false;
    }

    /**
     * @brief Heals owner for m_healPerTick HP (capped by Unit::heal()).
     * @return Message describing the heal.
     */
    std::string onTick(Unit &owner) override;

  private:
    int m_healPerTick;
};