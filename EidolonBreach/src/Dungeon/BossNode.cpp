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
                   const SummonRegistry *summonRegistry,
                   int floorIndex,
                   const ItemRegistry *itemRegistry)
    : EliteNode{
          std::move(populateEnemies), floorAffinity, dungeonEnemyLevel,
          summonRegistry, floorIndex, itemRegistry}
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

    applyFloorExposureModifier(party);

    for (std::size_t i = 0; i < party.size(); ++i)
    {
        Unit *u = party.getUnitAt(i);
        auto *pc{u ? u->asPlayableCharacter() : nullptr};
        if (pc)
            pc->modifyExposure(CombatConstants::kEliteExposureSpike);
    }

    runBattle(party, meta, runCtx, eventBus, renderer, input);
}

std::string BossNode::description() const
{
    return "[Boss] The breach guardian. Defeat it to complete the run.";
}