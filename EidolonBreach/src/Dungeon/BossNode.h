#pragma once
/**
 * @file BossNode.h
 * @brief Final map node. Applies an Exposure spike and ends the run on completion.
 */

#include "Dungeon/EliteNode.h"

class IRenderer;
class IInputHandler;

class BossNode : public EliteNode
{
  public:
    explicit BossNode(std::function<void(Party &)> populateEnemies,
                      Affinity floorAffinity = Affinity::Aether,
                      int dungeonEnemyLevel = 1,
                      const SummonRegistry *summonRegistry = nullptr);

    void enter(Party &party, MetaProgress &meta,
               RunContext &runCtx, EventBus &eventBus,
               IRenderer &renderer, IInputHandler &input) override;

    [[nodiscard]] std::string description() const override;
    [[nodiscard]] NodeType nodeType() const override
    {
        return NodeType::Boss;
    }
};