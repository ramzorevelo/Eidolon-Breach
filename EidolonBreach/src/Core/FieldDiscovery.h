#pragma once
/**
 * @file FieldDiscovery.h
 * @brief IDs for Field Discovery effects activated mid-run from vote ratio thresholds.
 *        Active discoveries are stored in RunContext and read by Battle.
 */
#include <string_view>

namespace FieldDiscoveryIds
{
/**
 * Molten Lattice — Blaze ≥ 60% + Terra ≥ 30% of cumulative votes.
 * Effect: Blaze RF trigger also applies Shield(15, 1) to all allies.
 */
constexpr std::string_view kMoltenLattice = "molten_lattice";

/**
 * Arctic Surge — Frost ≥ 60% + Tempest ≥ 30% of cumulative votes.
 * Effect: Frost RF trigger applies Slow for +1 extra duration (3 instead of 2).
 */
constexpr std::string_view kArcticSurge = "arctic_surge";

/**
 * Lattice Attunement — Aether ≥ 30% + any single affinity ≥ 50% of cumulative votes.
 * Effect: Aether RF trigger adds kAetherVoteFraction extra votes to every affinity.
 */
constexpr std::string_view kLatticeAttunement = "lattice_attunement";
} // namespace FieldDiscoveryIds