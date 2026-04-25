/**
 * @file BossNode.cpp
 * @brief BossNode implementation.
 */

#include "Dungeon/BossNode.h"
#include "Core/EventBus.h"
#include "Entities/Party.h"
#include "Core/CombatConstants.h"
#include "Entities/PlayableCharacter.h"
#include <iostream>

BossNode::BossNode(std::function<void(Party &)> populateEnemies,
                   Affinity floorAffinity,
                   const SummonRegistry *summonRegistry)
    : EliteNode{std::move(populateEnemies), floorAffinity, summonRegistry}
{
}

void BossNode::enter(Party &party,
                     MetaProgress &meta,
                     RunContext &runCtx,
                     EventBus &eventBus)
{
    std::cout << "\n=== BOSS ENCOUNTER ===\n"
              << "The final guardian of the breach awaits.\n"
              << "Press Enter to proceed...";
    std::cin.get();

    std::cout << "Exposure spikes by " << CombatConstants::kEliteExposureSpike
              << " for all party members!\n";

    for (std::size_t i{0}; i < party.size(); ++i)
    {
        Unit *u{party.getUnitAt(i)};
        if (auto *pc = dynamic_cast<PlayableCharacter *>(u))
            pc->modifyExposure(CombatConstants::kEliteExposureSpike);
    }

    BattleNode::enter(party, meta, runCtx, eventBus);
}

std::string BossNode::description() const
{
    return "[Boss] The breach guardian. Defeat it to complete the run.";
}