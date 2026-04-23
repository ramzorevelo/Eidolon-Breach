#pragma once
/**
 * @file EventNode.h
 * @brief Map node presenting a scripted event with a choice.
 *        Full event table and Bond Trial logic added in a later phase.
 */

#include "Dungeon/MapNode.h"

class EventNode : public MapNode
{
  public:
    void enter(Party &party, MetaProgress &meta,
               RunContext &runCtx, EventBus &eventBus) override;

    [[nodiscard]] std::string description() const override;
};