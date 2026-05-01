/**
 * @file TreasureNode.cpp
 * @brief TreasureNode implementation.
 */

#include "Dungeon/TreasureNode.h"
#include "Entities/Party.h"
#include "Vestiges/VestigeFactory.h"
#include <iostream>
#include <random>
#include <limits>

namespace
{
void offerVestigeDiscard(Party &party, std::unique_ptr<IVestige> incoming)
{
    std::cout << "\n  Party is at vestige capacity (" << Party::kMaxVestiges << ").\n"
              << "  Incoming: [" << incoming->getName() << "] — "
              << incoming->getDescription() << "\n\n"
              << "  Choose a vestige to discard, or [0] to keep your current loadout:\n";

    const auto &vestiges{party.getVestiges()};
    for (std::size_t i{0}; i < vestiges.size(); ++i)
        std::cout << "  [" << (i + 1) << "] " << vestiges[i]->getName()
                  << " — " << vestiges[i]->getDescription() << '\n';

    std::cout << "  Discard: ";
    int choice{0};
    std::cin >> choice;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    if (choice < 1 || choice > static_cast<int>(vestiges.size()))
    {
        std::cout << "  Kept your current vestiges. " << incoming->getName() << " was discarded.\n";
        return;
    }

    party.removeVestige(static_cast<std::size_t>(choice - 1));
    party.addVestige(std::move(incoming));
    std::cout << "  Vestige replaced.\n";
}
} // namespace

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

    auto overflow{party.addVestige(std::move(vestige))};
    if (overflow.has_value())
        offerVestigeDiscard(party, std::move(*overflow));
}

std::string TreasureNode::description() const
{
    return "[Treasure] Gold and a vestige await.";
}