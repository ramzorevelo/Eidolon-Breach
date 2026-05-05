#pragma once
/**
 * @file VentAction.h
 * @brief Vent action [V]: reduces Exposure to 0, ends the character's turn.
 */

#include "Actions/IAction.h"

/**
 * @brief 0 SP, 0 energy cost. Available when 0 < Exposure < 100.
 */
class VentAction : public IAction
{
  public:
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
    static const ActionData kVentData;
};