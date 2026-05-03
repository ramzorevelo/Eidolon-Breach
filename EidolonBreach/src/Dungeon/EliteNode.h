#pragma once
/**
 * @file EliteNode.h
 * @brief Map node with a tougher battle. Applies an Exposure spike before combat
 *        and stores a Lattice Impression stub on the enemy party.
 */

#include "Dungeon/BattleNode.h"

class EliteNode : public BattleNode
{
  public:
    explicit EliteNode(std::function<void(Party &)> populateEnemies,
                       Affinity floorAffinity = Affinity::Aether,
                       int dungeonEnemyLevel = 1,
                       const SummonRegistry *summonRegistry = nullptr);

    void enter(Party &party, MetaProgress &meta,
               RunContext &runCtx, EventBus &eventBus) override;

    [[nodiscard]] std::string description() const override;
};