/**
 * @file RegenEffect.cpp
 * @brief RegenEffect implementation.
 */

#include "Core/Effects/RegenEffect.h"
#include "Core/EffectIds.h"
#include "Entities/Unit.h"
#include <string>
#include <ostream>
RegenEffect::RegenEffect(int healPerTick, int duration)
    : StatusEffectBase{EffectIds::kRegen, "Regen", std::make_optional(duration), {EffectTags::kBuff, EffectTags::kHoT, EffectTags::kTerra}}, m_healPerTick{healPerTick}
{
}

std::string RegenEffect::onTick(Unit &owner)
{
    owner.heal(m_healPerTick);
    return owner.getName() + " regenerates " + std::to_string(m_healPerTick) + " HP.";
}