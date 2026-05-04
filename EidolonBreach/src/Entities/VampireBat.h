#pragma once
/**
 * @file VampireBat.h
 * @brief Vampire Bat: lifedrain every 3rd turn; Bloodless break effect.
 */
#include "Entities/Enemy.h"

/** Vampire Bat: lifedrain attack every 3rd turn. */
class VampireBat : public Enemy
{
  public:
    VampireBat(std::string name, int maxHp, int maxToughness);

    ActionResult performAttack() override;

    /**
     * @brief Activates Bloodless state for kBloodlessDuration turns.
     *        Called by onBreakCallback() when the Bloodless BreakEffect fires.
     */
    void activateBloodless();

    /** @brief Zeroes HP damage output for kBloodlessDuration turns. */
    void onBreakCallback() override;

    [[nodiscard]] std::string getIntentLabel() const override;

  private:
    int m_turnCount{0};
    bool m_bloodlessActive{false};
    int m_bloodlessTurnsRemaining{0};
    static constexpr int kBloodlessDuration{2};
};