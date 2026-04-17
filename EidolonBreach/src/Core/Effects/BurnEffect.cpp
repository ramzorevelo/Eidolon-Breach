/**
 * @file BurnEffect.cpp
 * @brief BurnEffect implementation.
 */

#include "Core/Effects/BurnEffect.h"
#include "Core/EffectIds.h"
#include "Entities/Unit.h"
#include <string>
#include <ostream>
BurnEffect::BurnEffect(int damagePerTick, int duration)
    : StatusEffectBase{EffectIds::kBurn, "Burn", std::make_optional(duration), {EffectTags::kDebuff, EffectTags::kDoT, EffectTags::kBlaze}}, m_damagePerTick{damagePerTick}
{
}

std::string BurnEffect::onTick(Unit &owner)
{
    // DoT deliberately bypasses shield absorption — use takeTrueDamage.
    owner.takeTrueDamage(m_damagePerTick);
    return owner.getName() + " takes " + std::to_string(m_damagePerTick) + " Burn damage!";
}