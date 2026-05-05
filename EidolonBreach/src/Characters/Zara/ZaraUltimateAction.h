#pragma once
/**
 * @file ZaraUltimateAction.h
 * @brief Arctic Shatter — Zara's Ultimate. Deals Frost damage and Slows all enemies.
 */

#include "Actions/IAction.h"

class ZaraUltimateAction : public IAction
{
  public:
    ZaraUltimateAction();

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
    static constexpr float kSlowRatio{0.40f};
    static constexpr int kSlowDuration{3};
    static constexpr int kToughnessDamage{20};
};