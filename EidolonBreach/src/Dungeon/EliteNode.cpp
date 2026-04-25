/**
 * @file EliteNode.cpp
 * @brief EliteNode implementation.
 */

#include "Dungeon/EliteNode.h"
#include "Core/CombatConstants.h"
#include "Entities/Party.h"
#include "Entities/PlayableCharacter.h"
#include "Entities/Unit.h"
#include "Vestiges/VestigeFactory.h"
#include <iostream>

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
        if (!party.addVestige(std::move(vestige)))
            std::cout << "Party is at vestige capacity. Vestige lost.\n";
    }
}

std::string EliteNode::description() const
{
    return "[Elite] Dangerous foe - Exposure spike on entry.";
}