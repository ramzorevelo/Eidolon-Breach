#pragma once
/**
 * @file ToughnessBreakerVestige.h
 * @brief Standard vestige: +20% Toughness damage against enemies weak to the action's affinity.
 */

#include "Vestiges/IVestige.h"

/**
 * @brief When an action hits an enemy that has a toughness weakness to the
 *        action's affinity (modifier > 1.0), applies an additional toughness
 *        hit equal to kBonusFraction of the action's base toughness damage.
 *        The bonus hit goes through applyToughnessHit(), so the affinity
 *        multiplier is applied a second time on the bonus portion.
 *        Net effect: total toughness damage = base * affinityMod * 1.2 against weak enemies.
 */
class ToughnessBreakerVestige : public IVestige
{
  public:
    void onAction(PlayableCharacter &actor,
                  ActionResult &result,
                  BattleState &state) override;

    [[nodiscard]] std::string getName() const override;
    [[nodiscard]] std::string getDescription() const override;

  private:
    static constexpr float kBonusFraction{0.20f};
    static constexpr float kWeaknessThreshold{1.0f};
};