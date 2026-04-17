/**
 * @file SlowEffect.cpp
 * @brief SlowEffect implementation.
 */

#include "Core/Effects/SlowEffect.h"
#include "Core/EffectIds.h"

SlowEffect::SlowEffect(float slowRatio, int duration)
    : StatusEffectBase{EffectIds::kSlow, "Slow", std::make_optional(duration), {EffectTags::kDebuff, EffectTags::kStatMod, EffectTags::kFrost}}, m_slowRatio{slowRatio}
{
}

Stats SlowEffect::modifyStatsFlat(Stats base) const
{
    base.spd -= static_cast<int>(static_cast<float>(base.spd) * m_slowRatio);
    return base;
}