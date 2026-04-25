#pragma once
/**
 * @file BossNode.h
 * @brief Final map node. Applies an Exposure spike and ends the run on completion.
 */

#include "Dungeon/EliteNode.h"

class BossNode : public EliteNode
{
  public:
    explicit BossNode(std::function<void(Party &)> populateEnemies,
                      Affinity floorAffinity = Affinity::Aether,
                      const SummonRegistry *summonRegistry = nullptr);

    void enter(Party &party, MetaProgress &meta,
               RunContext &runCtx, EventBus &eventBus) override;

    [[nodiscard]] std::string description() const override;
};