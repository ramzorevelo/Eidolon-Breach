#pragma once
/**
 * @file TreasureNode.h
 * @brief Map node that awards gold and a Common vestige.
 */

#include "Dungeon/MapNode.h"
#include <cstdint>

class TreasureNode : public MapNode
{
  public:
    explicit TreasureNode(int goldAmount = 30, std::uint32_t vestigeRngSeed = 0u);

    void enter(Party &party, MetaProgress &meta,
               RunContext &runCtx, EventBus &eventBus) override;

    [[nodiscard]] std::string description() const override;

  private:
    int m_goldAmount{30};
    std::uint32_t m_vestigeRngSeed{0u};
};