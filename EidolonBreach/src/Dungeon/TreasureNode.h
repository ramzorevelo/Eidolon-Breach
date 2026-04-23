#pragma once
/**
 * @file TreasureNode.h
 * @brief Map node that awards gold to the party.
 *        Full vestige award added when VestigeFactory is available.
 */

#include "Dungeon/MapNode.h"

class TreasureNode : public MapNode
{
  public:
    explicit TreasureNode(int goldAmount = 30);

    void enter(Party &party, MetaProgress &meta,
               RunContext &runCtx, EventBus &eventBus) override;

    [[nodiscard]] std::string description() const override;

  private:
    int m_goldAmount{30};
};