/**
 * @file EliteNode.cpp
 * @brief EliteNode implementation.
 */

#include "Dungeon/EliteNode.h"
#include "Vestiges/IVestige.h"
#include <limits>
#include <memory>
#include "Core/CombatConstants.h"
#include "Entities/Party.h"
#include "Entities/PlayableCharacter.h"
#include "Entities/Unit.h"
#include "Vestiges/VestigeFactory.h"
#include <iostream>

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

EliteNode::EliteNode(std::function<void(Party &)> populateEnemies,
                     Affinity floorAffinity,
                     const SummonRegistry *summonRegistry)
    : BattleNode{std::move(populateEnemies), floorAffinity, 20, summonRegistry}
{
}

void EliteNode::enter(Party &party,
                      MetaProgress &meta,
                      RunContext &runCtx,
                      EventBus &eventBus)
{
    std::cout << "\n=== ELITE ENCOUNTER ===\n"
              << "Exposure spikes by " << CombatConstants::kEliteExposureSpike
              << " for all party members!\n";

    for (std::size_t i{0}; i < party.size(); ++i)
    {
        Unit *u{party.getUnitAt(i)};
        if (auto *pc = dynamic_cast<PlayableCharacter *>(u))
            pc->modifyExposure(CombatConstants::kEliteExposureSpike);
    }

    BattleNode::enter(party, meta, runCtx, eventBus);

    // Award a Corrupted vestige for defeating an Elite.
    if (!party.isAllDead())
    {
        std::mt19937 rng{static_cast<std::uint32_t>(party.size())};
        auto vestige{VestigeFactory::makeRandom(VestigeFactory::Rarity::Corrupted, rng)};
        std::cout << "A Corrupted vestige manifests: [" << vestige->getName() << "]\n"
                  << vestige->getDescription() << '\n';
        auto overflow{party.addVestige(std::move(vestige))};
        if (overflow.has_value())
            offerVestigeDiscard(party, std::move(*overflow));
    }
}

std::string EliteNode::description() const
{
    return "[Elite] Dangerous foe - Exposure spike on entry.";
}