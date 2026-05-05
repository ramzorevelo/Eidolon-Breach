#pragma once
/**
 * @file VexUltimateAction.h
 * @brief Resonant Bulwark — Vex's Ultimate. Shields all allies and grants SP.
 */

#include "Actions/IAction.h"

class VexUltimateAction : public IAction
{
  public:
    VexUltimateAction();

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
    static constexpr int kShieldAmount{40};
    static constexpr int kShieldDuration{3};
    static constexpr int kSpGrant{30};
    static constexpr int kToughnessDamage{15};
};