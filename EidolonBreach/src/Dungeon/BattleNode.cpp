/**
 * @file BattleNode.cpp
 * @brief BattleNode implementation.
 */

#include "Dungeon/BattleNode.h"
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
#include <limits>

BattleNode::BattleNode(std::function<void(Party &)> populateEnemies,
                       Affinity floorAffinity,
                       int xpReward)
    : m_populateEnemies{std::move(populateEnemies)},
      m_floorAffinity{floorAffinity},
      m_xpReward{xpReward}
{
}

void BattleNode::enter(Party &party,
                       MetaProgress &meta,
                       RunContext &runCtx,
                       EventBus &eventBus)
{
    std::cout << "Press Enter to begin battle...";
    std::cin.get();

    Party enemyParty{};
    m_populateEnemies(enemyParty);
    applyFloorAffinityModifiers(enemyParty);

    ConsoleRenderer renderer{};
    ConsoleInputHandler inputHandler{};
    Battle battle{party, enemyParty, renderer, inputHandler,
                  runCtx, eventBus};
    battle.run();

    if (enemyParty.isAllDead())
    {
        for (std::size_t i{0}; i < party.size(); ++i)
        {
            Unit *u{party.getUnitAt(i)};
            if (u && u->isAlive())
                meta.gainXP(u->getId(), m_xpReward);
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