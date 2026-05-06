/**
 * @file BossNode.cpp
 * @brief BossNode implementation.
 */

#include "Dungeon/BossNode.h"
#include "Core/EventBus.h"
#include "Entities/Party.h"
#include "Core/CombatConstants.h"
#include "Entities/PlayableCharacter.h"
#include "UI/IRenderer.h"
#include "UI/IInputHandler.h"

BossNode::BossNode(std::function<void(Party &)> populateEnemies,
                   Affinity floorAffinity,
                   int dungeonEnemyLevel,
                   const SummonRegistry *summonRegistry)
    : EliteNode{
          std::move(populateEnemies), floorAffinity, dungeonEnemyLevel, summonRegistry}
{
}
void BossNode::enter(Party &party, MetaProgress &meta,
                     RunContext &runCtx, EventBus &eventBus,
                     IRenderer &renderer, IInputHandler &input)
{
    renderer.renderMessage("=== BOSS ENCOUNTER ===");
    renderer.renderMessage("The final guardian of the breach awaits.");
    renderer.renderMessage("Exposure spikes by " + std::to_string(CombatConstants::kEliteExposureSpike) + " for all party members!");
    renderer.presentPause(800);

    for (std::size_t i = 0; i < party.size(); ++i)
    {
        Unit *u = party.getUnitAt(i);
        if (auto *pc = dynamic_cast<PlayableCharacter *>(u))
            pc->modifyExposure(CombatConstants::kEliteExposureSpike);
    }

    runBattle(party, meta, runCtx, eventBus, renderer, input);
}

std::string BossNode::description() const
{
    return "[Boss] The breach guardian. Defeat it to complete the run.";
}