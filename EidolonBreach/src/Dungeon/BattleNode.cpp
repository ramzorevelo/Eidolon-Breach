/**
 * @file BattleNode.cpp
 * @brief BattleNode implementation.
 */

#include "UI/SDL3Renderer.h"
#include "Dungeon/BattleNode.h"
#include "Core/RunContext.h"
#include "Battle/Battle.h"
#include "Core/Affinity.h"
#include "Core/CombatConstants.h"
#include "Core/EventBus.h"
#include "Core/MetaProgress.h"
#include "Core/RunContext.h"
#include "Entities/Enemy.h"
#include "Entities/Party.h"
#include "Entities/PlayableCharacter.h"
#include "Entities/Unit.h"
#include "UI/ConsoleInputHandler.h"
#include "UI/ConsoleRenderer.h"
#include <iostream>
#include "Summons/SummonRegistry.h"
#include <cmath>
#include <limits>

BattleNode::BattleNode(std::function<void(Party &)> populateEnemies,
                       Affinity floorAffinity,
                       int dungeonEnemyLevel,
                       const SummonRegistry *summonRegistry)
    : m_populateEnemies{std::move(populateEnemies)},
      m_floorAffinity{floorAffinity},
      m_dungeonEnemyLevel{dungeonEnemyLevel},
      m_summonRegistry{summonRegistry}
{
}

void BattleNode::enter(Party &party,
                       MetaProgress &meta,
                       RunContext &runCtx,
                       EventBus &eventBus)
{
    std::cout << "Press Enter to begin battle...";
    std::cin.get();
    runBattle(party, meta, runCtx, eventBus);
}

void BattleNode::runBattle(Party &party,
                           MetaProgress &meta,
                           RunContext &runCtx,
                           EventBus &eventBus)
{
    Party enemyParty{};
    m_populateEnemies(enemyParty);
    applyFloorAffinityModifiers(enemyParty);

    //ConsoleRenderer renderer{};
    SDL3Renderer renderer{"Eidolon Breach", 1280, 720};
    ConsoleInputHandler inputHandler{};
    Battle battle{party, enemyParty, renderer, inputHandler,
                  runCtx, eventBus, nullptr, m_summonRegistry};
    battle.run();

    if (enemyParty.isAllDead() && runCtx.runMode == RunMode::Classic)
    {
        for (std::size_t i{0}; i < party.size(); ++i)
        {
            Unit *u{party.getUnitAt(i)};
            if (!u || !u->isAlive())
                continue;

            const int charLevel{
                meta.characterLevels.count(u->getId()) > 0
                    ? meta.characterLevels.at(u->getId())
                    : 1};
            const float gap{static_cast<float>(m_dungeonEnemyLevel - charLevel)};
            const float scale{std::clamp(
                1.0f + gap * CombatConstants::kCharBattleXpLevelScale,
                0.1f, 2.0f)};
            const int xp{static_cast<int>(
                static_cast<float>(m_dungeonEnemyLevel) *
                CombatConstants::kCharBattleXpMultiplier * scale)};

            const int newLevel{meta.gainXP(u->getId(), xp)};

            auto *pc{dynamic_cast<PlayableCharacter *>(u)};
            if (!pc)
                continue;

            pc->applyUnlocks(newLevel);
        }
    }
}

std::string BattleNode::description() const
{
    return "[Battle] Enemies ahead.";
}

void BattleNode::applyFloorAffinityModifiers(Party &enemyParty) const
{
    if (m_floorAffinity == Affinity::Aether)
        return;

    const Affinity opponent{getAffinityOpponent(m_floorAffinity)};

    for (std::size_t i{0}; i < enemyParty.size(); ++i)
    {
        Unit *u{enemyParty.getUnitAt(i)};
        if (!u)
            continue;
        auto *e{dynamic_cast<Enemy *>(u)};
        if (!e)
            continue;

        if (u->getAffinity() == m_floorAffinity)
            e->scaleMaxToughness(1.0f + CombatConstants::kFloorAffinityToughnessBonus);
        else if (u->getAffinity() == opponent)
            e->scaleMaxToughness(1.0f - CombatConstants::kFloorAffinityToughnessBonus);
    }
}