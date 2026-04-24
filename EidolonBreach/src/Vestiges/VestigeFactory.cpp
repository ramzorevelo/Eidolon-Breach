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

std::unique_ptr<IVestige> VestigeFactory::makeRandom(Rarity rarity, std::mt19937 &rng)
{
    if (rarity == Rarity::Corrupted)
    {
        std::uniform_int_distribution<int> dist{0, 1};
        return dist(rng) == 0
                   ? std::unique_ptr<IVestige>{std::make_unique<VestigeOfTheUnbound>()}
                   : std::unique_ptr<IVestige>{std::make_unique<AttunistGambitVestige>()};
    }

    std::uniform_int_distribution<int> dist{0, 2};
    switch (dist(rng))
    {
    case 0:
        return std::make_unique<FlameResonanceVestige>();
    case 1:
        return std::make_unique<ToughnessBreakerVestige>();
    default:
        return std::make_unique<EchoingStrikeVestige>();
    }
}