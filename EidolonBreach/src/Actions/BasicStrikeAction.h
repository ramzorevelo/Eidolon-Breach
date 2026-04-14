#pragma once

/**
 * @file BasicStrikeAction.h
 * @brief Standard single‑target physical attack.
 */

#include "Actions/IAction.h"

/** Basic attack: costs 0 SP, grants +15 SP to party and +8 Energy to self. */
class BasicStrikeAction : public IAction
{
  public:
    std::string label() const override;
    bool isAvailable(const PlayableCharacter &user, const Party &party) const override;
    ActionResult execute(PlayableCharacter &user,
                         Party &allies,
                         Party &enemies,
                         std::optional<TargetInfo> target) override;
    Affinity getAffinity() const override
    {
        return Affinity::Blaze;
    } // example
};