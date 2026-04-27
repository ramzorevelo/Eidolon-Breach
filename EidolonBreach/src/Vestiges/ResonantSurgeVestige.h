#pragma once
/**
 * @file ResonantSurgeVestige.h
 * @brief Standard vestige: after the Resonance Field triggers,
 *        the party gains +20 SP.
 */
#include "Vestiges/IVestige.h"

class ResonantSurgeVestige : public IVestige
{
  public:
    void onBattleStart(Battle &battle, BattleState &state) override;

    [[nodiscard]] std::string getName() const override;
    [[nodiscard]] std::string getDescription() const override;

  private:
    static constexpr int kSpBonus{20};
};