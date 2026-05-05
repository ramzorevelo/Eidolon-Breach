#pragma once
/**
 * @file VexBulwarkAction.h
 * @brief Earthen Shield — Vex's Slot Skill [1].
 *        Applies a Terra shield to one ally. SP cost: 25.
 */
#include "Actions/IAction.h"

class VexBulwarkAction : public IAction
{
  public:
    VexBulwarkAction();

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
    static constexpr int kShieldAmount{30};
    static constexpr int kShieldDuration{2};
};