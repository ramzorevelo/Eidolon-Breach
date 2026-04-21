#pragma once
/**
 * @file TargetingHeuristics.h
 * @brief Auto-select targeting heuristics for ally-targeting skills (GDD §4.12).
 *
 * Called by PlayableCharacter::selectTarget() when the action's TargetMode
 * requires an ally and no explicit player override has been made.
 * The player may override these defaults with left/right arrow (Phase 3+ SDL).
 */

#include "Core/TargetInfo.h"
#include "Core/TargetMode.h"
#include <cstddef>

class Party;

namespace TargetingHeuristics
{
/**
 * @brief Returns the default target index within the ally party for the
 *        given targeting mode.
 *
 * - Heals (ConsumableHeal / Regen)       → lowest current HP ally index.
 * - Shields (ShieldEffect application)   → lowest max HP ally index.
 * - ATK buffs (StatModifier ATK)          → highest ATK stat ally index.
 * - All other modes                       → index 0 (first alive ally).
 *
 * @param mode   The TargetMode from ActionData.
 * @param allies The ally party to search.
 * @return 0-based index into the alive units of `allies`.
 */
[[nodiscard]] std::size_t defaultAllyTarget(TargetMode mode,
                                            const Party &allies);
} // namespace TargetingHeuristics