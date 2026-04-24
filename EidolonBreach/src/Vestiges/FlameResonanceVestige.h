#pragma once
/**
 * @file FlameResonanceVestige.h
 * @brief Standard vestige: Blaze actions add +5 to the Resonance Field gauge.
 */

#include "Vestiges/IVestige.h"

/**
 * @brief Each Blaze action contributes an additional kBlazeBonus gauge points
 *        to the Resonance Field, independent of the acting character's
 *        resonanceContribution stat.
 */
class FlameResonanceVestige : public IVestige
{
  public:
    void onAction(PlayableCharacter &actor,
                  ActionResult &result,
                  BattleState &state) override;

    [[nodiscard]] std::string getName() const override;
    [[nodiscard]] std::string getDescription() const override;

  private:
    static constexpr int kBlazeBonus{5};
};