#pragma once
/**
 * @file UltimateAction.h
 * @brief Ultimate [R]: requires full Momentum (100), resets to 0, refunds 5.
 */

#include "Actions/IAction.h"

/** Ultimate: available only when Momentum = 100. Resets Momentum to 0, refunds 5. */
class UltimateAction : public IAction
{
  public:
    UltimateAction();

    std::string label() const override;
    bool isAvailable(const PlayableCharacter &user,
                     const Party &party) const override;
    ActionResult execute(PlayableCharacter &user,
                         Party &allies,
                         Party &enemies,
                         std::optional<TargetInfo> target) override;
    Affinity getAffinity() const override;
    const ActionData &getActionData() const override;

  private:
    ActionData m_data;
    static constexpr int kEnergyRefund{5};
};