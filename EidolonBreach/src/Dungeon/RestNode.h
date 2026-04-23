#pragma once
/**
 * @file RestNode.h
 * @brief Map node offering Heal, Purge, and (Draft) Attune actions.
 *        Each action is independent and optional.
 */

#include "Dungeon/MapNode.h"

class RestNode : public MapNode
{
  public:
    void enter(Party &party, MetaProgress &meta,
               RunContext &runCtx, EventBus &eventBus) override;

    [[nodiscard]] std::string description() const override;

  private:
    /** @brief Restore partial HP to all alive party members. */
    void applyHeal(Party &party) const;

    /** @brief Reduce all party members' Exposure by kPurgeExposureReduction. */
    void applyPurge(Party &party) const;
};