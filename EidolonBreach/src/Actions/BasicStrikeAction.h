#pragma once
/**
 * @file BasicStrikeAction.h
 * @brief Basic Attack [Q]: free action, grants +15 SP (via result.spGained),
 *        +25 Energy to self.
 */

#include "Actions/IAction.h"

/**
 * @brief Basic Attack: no SP cost, no cooldown.
 * Sets result.spGained = kSpGainToParty; Battle applies it to the party pool.
 * Grants kEnergyGainToUser Energy directly to the acting character.
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
    static constexpr int kEnergyGainToUser{25};
};