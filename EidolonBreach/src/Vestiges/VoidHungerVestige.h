#pragma once
/**
 * @file VoidHungerVestige.h
 * @brief Corrupted vestige: +8 Exposure each turn start. Damage actions
 *        heal all allies for 5% of the damage dealt (rounded down, minimum 0).
 */
#include "Vestiges/IVestige.h"

class VoidHungerVestige : public IVestige
{
  public:
    void onTurnStart(PlayableCharacter &activeCharacter, BattleState &state) override;
    void onAction(PlayableCharacter &actor, ActionResult &result,
                  BattleState &state) override;

    [[nodiscard]] std::string getName() const override;
    [[nodiscard]] std::string getDescription() const override;

  private:
    static constexpr int kExposurePerTurn{8};
    static constexpr int kHealDivisor{20}; ///< result.value / 20 = 5% of damage.
};