#pragma once
/**
 * @file EffectIds.h
 * @brief Compile-time string constants for all status effect IDs and tags.
 *
 * Every applyEffect() / hasEffect() call site must use these constants.
 * Using raw literals risks silent typo bugs that only surface at runtime.
 */

#include <string_view>

namespace EffectIds
{
constexpr std::string_view kBurn = "burn";
constexpr std::string_view kSlow = "slow";
constexpr std::string_view kShield = "shield";
constexpr std::string_view kRegen = "regen";
} // namespace EffectIds

namespace EffectTags
{
constexpr std::string_view kDebuff = "Debuff";
constexpr std::string_view kBuff = "Buff";
constexpr std::string_view kDoT = "DoT";
constexpr std::string_view kHoT = "HoT";
constexpr std::string_view kStatMod = "StatMod";
constexpr std::string_view kShield = "Shield";
constexpr std::string_view kBlaze = "Blaze";
constexpr std::string_view kFrost = "Frost";
constexpr std::string_view kTerra = "Terra";
} // namespace EffectTags