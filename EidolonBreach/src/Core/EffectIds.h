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
constexpr std::string_view kTempest = "Tempest";
constexpr std::string_view kTerra = "Terra";
constexpr std::string_view kAether = "Aether";
} // namespace EffectTags

// ---------------------------------------------------------------------------
// EffectTagFlag — bitmask representation of EffectTags for O(1) queries.
// Mirrors the string constants in EffectTags above; new tags get new bits here.
// ---------------------------------------------------------------------------
enum class EffectTagFlag : uint16_t
{
    None = 0,
    Debuff = 1 << 0,
    Buff = 1 << 1,
    DoT = 1 << 2,
    HoT = 1 << 3,
    Shield = 1 << 4,
    StatMod = 1 << 5,
    Blaze = 1 << 6,
    Frost = 1 << 7,
    Tempest = 1 << 8,
    Terra = 1 << 9,
    Aether = 1 << 10,
    // bits 11-15 reserved
};

/** Maps an EffectTags string constant to its EffectTagFlag bit.
 *  Returns 0 for unknown tags — safe to OR into the cache without effect. */
[[nodiscard]] inline uint16_t tagToFlag(std::string_view tag) noexcept
{
    if (tag == EffectTags::kDebuff)
        return static_cast<uint16_t>(EffectTagFlag::Debuff);
    if (tag == EffectTags::kBuff)
        return static_cast<uint16_t>(EffectTagFlag::Buff);
    if (tag == EffectTags::kDoT)
        return static_cast<uint16_t>(EffectTagFlag::DoT);
    if (tag == EffectTags::kHoT)
        return static_cast<uint16_t>(EffectTagFlag::HoT);
    if (tag == EffectTags::kShield)
        return static_cast<uint16_t>(EffectTagFlag::Shield);
    if (tag == EffectTags::kStatMod)
        return static_cast<uint16_t>(EffectTagFlag::StatMod);
    if (tag == EffectTags::kBlaze)
        return static_cast<uint16_t>(EffectTagFlag::Blaze);
    if (tag == EffectTags::kFrost)
        return static_cast<uint16_t>(EffectTagFlag::Frost);
    if (tag == EffectTags::kTempest)
        return static_cast<uint16_t>(EffectTagFlag::Tempest);
    if (tag == EffectTags::kTerra)
        return static_cast<uint16_t>(EffectTagFlag::Terra);
    if (tag == EffectTags::kAether)
        return static_cast<uint16_t>(EffectTagFlag::Aether);
    return 0;
}