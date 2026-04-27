/**
 * @file VestigeFactory.cpp
 * @brief VestigeFactory implementation.
 */

#include "Vestiges/VestigeFactory.h"
#include "Vestiges/AttunistGambitVestige.h"
#include "Vestiges/EchoingStrikeVestige.h"
#include "Vestiges/FlameResonanceVestige.h"
#include "Vestiges/ToughnessBreakerVestige.h"
#include "Vestiges/VestigeOfTheUnbound.h"
#include "Vestiges/ResonantSurgeVestige.h"
#include "Vestiges/SwiftStrikeVestige.h"
#include "Vestiges/VoidHungerVestige.h"

std::unique_ptr<IVestige> VestigeFactory::makeRandom(Rarity rarity, std::mt19937 &rng)
{
    if (rarity == Rarity::Corrupted)
    {
        std::uniform_int_distribution<int> dist{0, 2};
        switch (dist(rng))
        {
        case 0:
            return std::make_unique<VestigeOfTheUnbound>();
        case 1:
            return std::make_unique<AttunistGambitVestige>();
        default:
            return std::make_unique<VoidHungerVestige>();
        }
    }

    std::uniform_int_distribution<int> dist{0, 4};
    switch (dist(rng))
    {
    case 0:
        return std::make_unique<FlameResonanceVestige>();
    case 1:
        return std::make_unique<ToughnessBreakerVestige>();
    case 2:
        return std::make_unique<EchoingStrikeVestige>();
    case 3:
        return std::make_unique<SwiftStrikeVestige>();
    default:
        return std::make_unique<ResonantSurgeVestige>();
    }
}