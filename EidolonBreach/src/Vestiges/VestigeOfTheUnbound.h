#pragma once
/**
 * @file VestigeOfTheUnbound.h
 * @brief Corrupted vestige: the bearer gains +5 Exposure at the start of each of their turns.
 *        Fractured characters still gain Exposure. Re-hitting 100 refreshes Breachborn.
 */

#include "Vestiges/IVestige.h"

class VestigeOfTheUnbound : public IVestige
{
  public:
    void onTurnStart(PlayableCharacter &activeCharacter, BattleState &state) override;

    [[nodiscard]] std::string getName() const override;
    [[nodiscard]] std::string getDescription() const override;

  private:
    static constexpr int kExposurePerTurn{5};
};