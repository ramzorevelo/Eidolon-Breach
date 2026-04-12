#pragma once
#include "Core/CombatConstants.h"
#include <algorithm>

namespace CombatUtils
{
    /// @brief Calculate HP damage using DEF reduction formula.
    /// @param basePower Raw damage before mitigation
    /// @param defenderDef Defender's DEF stat
    /// @return Final damage (minimum 1)
    inline int calculateDamage(int basePower, int defenderDef)
    {
        float def = static_cast<float>(defenderDef);
        float K = CombatConstants::kDefScalingK;
        return std::max(1, static_cast<int>(basePower * (1.0f - def / (def + K))));
    }
}