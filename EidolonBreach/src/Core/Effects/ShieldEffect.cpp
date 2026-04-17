/**
 * @file ShieldEffect.cpp
 * @brief ShieldEffect implementation.
 */

#include "Core/Effects/ShieldEffect.h"
#include "Core/EffectIds.h"
#include <algorithm>

ShieldEffect::ShieldEffect(int absorbAmount, int duration)
    : StatusEffectBase{EffectIds::kShield, "Shield", std::make_optional(duration), {EffectTags::kBuff, EffectTags::kShield, EffectTags::kTerra}}, m_remainingAbsorb{absorbAmount}
{
}

int ShieldEffect::absorbDamage(int incoming)
{
    int absorbed{std::min(incoming, m_remainingAbsorb)};
    m_remainingAbsorb -= absorbed;
    return incoming - absorbed; // overflow hits HP
}

bool ShieldEffect::isExhausted() const
{
    return m_remainingAbsorb <= 0;
}