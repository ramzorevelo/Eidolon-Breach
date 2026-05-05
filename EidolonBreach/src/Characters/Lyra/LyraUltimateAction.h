#pragma once
/**
 * @file LyraUltimateAction.h
 * @brief Pyre's Ruin — Lyra's Ultimate. High Blaze single-target damage + Burn.
 */

#include "Actions/IAction.h"

class LyraUltimateAction : public IAction
{
  public:
    LyraUltimateAction();

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
    static constexpr int kBurnDamage{15};
    static constexpr int kBurnDuration{3};
};