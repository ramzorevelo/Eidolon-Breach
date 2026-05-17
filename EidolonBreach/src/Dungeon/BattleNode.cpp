/**
 * @file BattleNode.cpp
 * @brief BattleNode implementation.
 */

#include "Dungeon/BattleNode.h"
#include "Core/RunContext.h"
#include "Battle/Battle.h"
#include "Core/Affinity.h"
#include "Core/CombatConstants.h"
#include "Core/EventBus.h"
#include "Core/MetaProgress.h"
#include "Entities/Enemy.h"
#include "Entities/Party.h"
#include "Items/ItemRegistry.h"
#include "Entities/PlayableCharacter.h"
#include "Entities/Unit.h"
#include "Summons/SummonRegistry.h"
#include <cmath>
#include <limits>

namespace
{
constexpr int kBattleEntryPauseMs{400};
} // namespace

BattleNode::BattleNode(std::function<void(Party &)> populateEnemies,
                       Affinity floorAffinity,
                       int dungeonEnemyLevel,
                       const SummonRegistry *summonRegistry,
                       int floorIndex,
                       const ItemRegistry *itemRegistry,
                       DungeonDifficulty difficulty)
    : m_populateEnemies{std::move(populateEnemies)},
      m_floorAffinity{floorAffinity},
      m_dungeonEnemyLevel{dungeonEnemyLevel},
      m_summonRegistry{summonRegistry},
      m_floorIndex{floorIndex},
      m_itemRegistry{itemRegistry},
      m_difficulty{difficulty}
{
}

void BattleNode::enter(Party &party, MetaProgress &meta,
                       RunContext &runCtx, EventBus &eventBus,
                       IRenderer &renderer, IInputHandler &input)
{
    renderer.renderMessage("Entering battle...");
    renderer.presentPause(kBattleEntryPauseMs);
    applyFloorExposureModifier(party);
    runBattle(party, meta, runCtx, eventBus, renderer, input);
}

void BattleNode::runBattle(Party &party, MetaProgress &meta,
                           RunContext &runCtx, EventBus &eventBus,
                           IRenderer &renderer, IInputHandler &input)
{
    Party enemyParty{};
    m_populateEnemies(enemyParty);
    applyFloorAffinityModifiers(enemyParty);
    applyDifficultyScaling(enemyParty);

    Battle battle{party, enemyParty, renderer, input,
                  runCtx, eventBus, nullptr, m_summonRegistry, m_itemRegistry};
    try
    {
        battle.run();
    }
    catch (...)
    {
        renderer.clearBattleCache();
        throw;
    }
    if (enemyParty.isAllDead())
    {
        awardBattleXp(party, meta, runCtx);
        renderer.renderMessage("Battle complete. Press Enter to continue.");
        input.getMenuChoice(1);
    }
    renderer.clearBattleCache();
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

        if (u->getAffinity() == m_floorAffinity)
            u->scaleMaxToughness(1.0f + CombatConstants::kFloorAffinityToughnessBonus);
        else if (u->getAffinity() == opponent)
            u->scaleMaxToughness(1.0f - CombatConstants::kFloorAffinityToughnessBonus);
    }
}

void BattleNode::awardBattleXp(Party &party, MetaProgress &meta,
                               const RunContext &runCtx) const
{
    if (runCtx.runMode != RunMode::Classic)
        return;

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

        auto *pc{u->asPlayableCharacter()};
        if (pc)
            pc->applyUnlocks(newLevel);
    }
}

void BattleNode::applyFloorExposureModifier(Party &party) const
{
    int modifier{CombatConstants::kFloorDepthExposureModifier};
    if (m_difficulty == DungeonDifficulty::Hard)
        modifier = CombatConstants::kFloorDepthExposureModifierHard;
    else if (m_difficulty == DungeonDifficulty::Nightmare)
        modifier = CombatConstants::kFloorDepthExposureModifierNightmare;

    for (std::size_t i{0}; i < party.size(); ++i)
    {
        Unit *u{party.getUnitAt(i)};
        auto *pc{u ? u->asPlayableCharacter() : nullptr};
        if (pc && pc->isAlive())
            pc->modifyExposure(m_floorIndex * modifier);
    }
}

void BattleNode::applyDifficultyScaling(Party &enemyParty) const
{
    if (m_difficulty == DungeonDifficulty::Normal)
        return;

    const float hpScale{m_difficulty == DungeonDifficulty::Hard
                            ? CombatConstants::kHardEnemyHpScale
                            : CombatConstants::kNightmareEnemyHpScale};
    const float atkScale{m_difficulty == DungeonDifficulty::Hard
                             ? CombatConstants::kHardEnemyAtkScale
                             : CombatConstants::kNightmareEnemyAtkScale};

    for (std::size_t i{0}; i < enemyParty.size(); ++i)
    {
        Unit *u{enemyParty.getUnitAt(i)};
        if (u)
            u->scaleStats(hpScale, atkScale);
    }
}