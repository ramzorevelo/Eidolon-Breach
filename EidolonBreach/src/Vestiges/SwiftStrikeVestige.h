#pragma once
/**
 * @file SwiftStrikeVestige.h
 * @brief Standard vestige: if the PC's HP is full at the start of their turn,
 *        they gain +5 Energy.
 */
#include "Vestiges/IVestige.h"

class SwiftStrikeVestige : public IVestige
{
  public:
    void onTurnStart(PlayableCharacter &activeCharacter, BattleState &state) override;

    [[nodiscard]] std::string getName() const override;
    [[nodiscard]] std::string getDescription() const override;

  private:
    static constexpr int kEnergyBonus{5};
};