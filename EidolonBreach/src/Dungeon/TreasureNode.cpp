/**
 * @file TreasureNode.cpp
 * @brief TreasureNode implementation.
 */

#include "Dungeon/TreasureNode.h"
#include "Entities/Party.h"
#include "Vestiges/VestigeFactory.h"
#include <iostream>
#include <random>

TreasureNode::TreasureNode(int goldAmount, std::uint32_t vestigeRngSeed)
    : m_goldAmount{goldAmount}, m_vestigeRngSeed{vestigeRngSeed}
{
}

void TreasureNode::enter(Party &party,
                         MetaProgress & /*meta*/,
                         RunContext & /*runCtx*/,
                         EventBus & /*eventBus*/)
{
    party.getInventory().gold += m_goldAmount;
    std::cout << "\n=== TREASURE ===\n"
              << "Found " << m_goldAmount << " gold. "
              << "Party gold: " << party.getInventory().gold << '\n';

    std::mt19937 rng{m_vestigeRngSeed};
    auto vestige{VestigeFactory::makeRandom(VestigeFactory::Rarity::Common, rng)};
    std::cout << "A vestige resonates: [" << vestige->getName() << "]\n"
              << vestige->getDescription() << '\n';

    if (!party.addVestige(std::move(vestige)))
    {
        std::cout << "Party is at vestige capacity (" << Party::kMaxVestiges
                  << "). Vestige lost.\n";
    }
}

std::string TreasureNode::description() const
{
    return "[Treasure] Gold and a vestige await.";
}