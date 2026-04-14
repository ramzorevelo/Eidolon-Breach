#pragma once

/**
 * @file UltimateAction.h
 * @brief Ultimate ability that consumes all Energy for massive damage.
 */

#include "Actions/IAction.h"

/** Ultimate action: requires full Energy, resets it, grants +5 Energy back. */
class UltimateAction : public IAction
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
    }

  private:
    static constexpr int kEnergyRefund{5};
    static constexpr int kBasePower{60};
};