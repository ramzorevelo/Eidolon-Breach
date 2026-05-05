#pragma once
/**
 * @file VestigeFactory.h
 * @brief Creates Vestige instances by rarity tier.
 */

#include "Vestiges/IVestige.h"
#include <memory>
#include <random>

class VestigeFactory
{
  public:
    enum class Rarity
    {
        Common,   ///< Standard vestiges: no ongoing cost.
        Corrupted ///< Corrupted vestiges: powerful but impose Exposure per turn.
    };

    /**
     * @brief Construct a random vestige of the given rarity.
     *        Uses the provided RNG so callers control reproducibility.
     * @param rarity Tier to draw from.
     * @param rng    Seeded Mersenne Twister from the caller.
     * @return A freshly constructed vestige of the requested rarity.
     */
    [[nodiscard]] static std::unique_ptr<IVestige> makeRandom(Rarity rarity,
                                                              std::mt19937 &rng);
};