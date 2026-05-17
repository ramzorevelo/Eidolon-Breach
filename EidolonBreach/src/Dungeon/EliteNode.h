#pragma once
/**
 * @file EliteNode.h
 * @brief Map node with a tougher battle. Applies an Exposure spike before combat
 *        and stores a Lattice Impression stub on the enemy party.
 */

#include "Dungeon/BattleNode.h"

class IRenderer;
class IInputHandler;

class EliteNode : public BattleNode
{
  public:
    explicit EliteNode(std::function<void(Party &)> populateEnemies,
                       Affinity floorAffinity = Affinity::Aether,
                       int dungeonEnemyLevel = 1,
                       const SummonRegistry *summonRegistry = nullptr,
                       int floorIndex = 0,
                       const ItemRegistry *itemRegistry = nullptr,
                       DungeonDifficulty difficulty =
                           DungeonDifficulty::Normal);

    void enter(Party &party, MetaProgress &meta,
               RunContext &runCtx, EventBus &eventBus,
               IRenderer &renderer, IInputHandler &input) override;

    [[nodiscard]] std::string description() const override;
    [[nodiscard]] NodeType nodeType() const override
    {
        return NodeType::Elite;
    }
};