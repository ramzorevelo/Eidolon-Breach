#pragma once
/**
 * @file ZaraFrostbindAction.h
 * @brief Frostbind — Zara's Slot Skill [1].
 *        Slows one enemy and deals light toughness damage. SP cost: 25.
 */
#include "Actions/IAction.h"

class ZaraFrostbindAction : public IAction
{
  public:
    ZaraFrostbindAction();

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
    static constexpr int kSpCost{25};
    static constexpr float kSlowRatio{0.30f};
    static constexpr int kSlowDuration{2};
    static constexpr int kToughnessDamage{5};
};