/**
 * @file EliteNode.cpp
 * @brief EliteNode implementation.
 */

#include "Dungeon/EliteNode.h"
#include "Core/CombatConstants.h"
#include "Entities/Party.h"
#include "Entities/PlayableCharacter.h"
#include "Entities/Unit.h"
#include <iostream>

EliteNode::EliteNode(std::function<void(Party &)> populateEnemies,
                     Affinity floorAffinity)
    : BattleNode{std::move(populateEnemies), floorAffinity, 20}
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
}

std::string EliteNode::description() const
{
    return "[Elite] Dangerous foe - Exposure spike on entry.";
}