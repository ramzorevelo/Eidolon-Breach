#pragma once
/**
 * @file EchoingStrikeVestige.h
 * @brief Standard vestige: next SP-costing action is free after Resonance Field triggers.
 */

#include "Vestiges/IVestige.h"

/**
 * @brief Subscribes to ResonanceFieldTriggeredEvent in onBattleStart.
 *        When the event fires, sets m_nextActionFree = true.
 *        On the next onAction call where result.spCost > 0,
 *        refunds the SP cost to the party pool and clears the flag.
 *        m_nextActionFree resets at the start of each battle.
 */
class EchoingStrikeVestige : public IVestige
{
  public:
    void onBattleStart(Battle &battle, BattleState &state) override;
    void onAction(PlayableCharacter &actor,
                  ActionResult &result,
                  BattleState &state) override;

    [[nodiscard]] std::string getName() const override;
    [[nodiscard]] std::string getDescription() const override;

    /** @return true when the SP refund is armed and waiting for the next Slot Skill. */
    [[nodiscard]] bool isNextActionFree() const
    {
        return m_nextActionFree;
    }

  private:
    bool m_nextActionFree{false};
};