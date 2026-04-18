#pragma once
/**
 * @file BasicStrikeAction.h
 * @brief Basic Attack [Q]: free action, grants +15 SP to party, +25 Momentum to self.
 */

#include "Actions/IAction.h"

/**
 * @brief Basic Attack: costs 0 SP, grants +15 SP to party and +25 Momentum to user.
 * Always available.
 */
class BasicStrikeAction : public IAction
{
  public:
    BasicStrikeAction();

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
    static constexpr int kSpGainToParty{15};
};