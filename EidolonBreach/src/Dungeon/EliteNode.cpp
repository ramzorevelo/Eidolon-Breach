/**
 * @file EliteNode.cpp
 * @brief EliteNode implementation.
 */
#include "Dungeon/VestigeOffer.h"
#include "Dungeon/EliteNode.h"
#include "Vestiges/IVestige.h"
#include <limits>
#include <memory>
#include "Core/CombatConstants.h"
#include "Entities/Party.h"
#include "Core/RunContext.h"
#include "Entities/PlayableCharacter.h"
#include "Entities/Unit.h"
#include "Vestiges/VestigeFactory.h"
#include <chrono>
#include "UI/IRenderer.h"
#include "UI/IInputHandler.h"   

EliteNode::EliteNode(std::function<void(Party &)> populateEnemies,
                     Affinity floorAffinity,
                     int dungeonEnemyLevel,
                     const SummonRegistry *summonRegistry,
                     int floorIndex,
                     const ItemRegistry *itemRegistry,
                     DungeonDifficulty difficulty)
    : BattleNode{
          std::move(populateEnemies), floorAffinity, dungeonEnemyLevel,
          summonRegistry, floorIndex, itemRegistry, difficulty}
{
}

void EliteNode::enter(Party &party, MetaProgress &meta,
                      RunContext &runCtx, EventBus &eventBus,
                      IRenderer &renderer, IInputHandler &input)
{
    const int exposureSpike{
        m_difficulty == DungeonDifficulty::Hard
            ? CombatConstants::kEliteExposureSpikeHard
            : (m_difficulty == DungeonDifficulty::Nightmare
                   ? CombatConstants::kEliteExposureSpikeNightmare
                   : CombatConstants::kEliteExposureSpike)};

    renderer.renderMessage("=== ELITE ENCOUNTER ===");
    renderer.renderMessage("Exposure spikes by " +
                           std::to_string(exposureSpike) +
                           " for all party members!");
    renderer.presentPause(600);

    applyFloorExposureModifier(party);

    for (std::size_t i = 0; i < party.size(); ++i)
    {
        Unit *u = party.getUnitAt(i);
        auto *pc{u ? u->asPlayableCharacter() : nullptr};
        if (pc)
            pc->modifyExposure(exposureSpike);
    }

    runBattle(party, meta, runCtx, eventBus, renderer, input);

    // Vestiges are Labyrinth-only. Classic awards equipment drops instead.
    if (!party.isAllDead() && runCtx.runMode != RunMode::Classic)
    {
        std::mt19937 rng{static_cast<std::uint32_t>(
            std::chrono::steady_clock::now().time_since_epoch().count())};
        auto vestige = VestigeFactory::makeRandom(
            VestigeFactory::Rarity::Corrupted, rng);
        renderer.renderMessage("A Corrupted vestige resonates: [" +
                               vestige->getName() + "]");
        renderer.renderMessage(vestige->getDescription());
        renderer.presentPause(400);

        auto overflow = party.addVestige(std::move(vestige));
        if (overflow.has_value())
            DungeonHelpers::offerVestigeDiscard(
                party, std::move(*overflow), renderer, input);
    }
}

std::string EliteNode::description() const
{
    return "[Elite] Dangerous foe - Exposure spike on entry.";
}