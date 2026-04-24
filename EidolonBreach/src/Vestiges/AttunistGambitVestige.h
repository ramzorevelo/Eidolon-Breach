#pragma once
/**
 * @file AttunistGambitVestige.h
 * @brief Corrupted vestige: kills reduce killer Exposure by kExposureOnKill (capped at
 *        kMaxReductionPerBattle per battle). All Exposure gains are amplified by
 *        kExposureGainMultiplier.
 */

#include "Vestiges/IVestige.h"

class AttunistGambitVestige : public IVestige
{
  public:
    void onBattleStart(Battle &battle, BattleState &state) override;
    void onAction(PlayableCharacter &actor,
                  ActionResult &result,
                  BattleState &state) override;
    void onBattleEnd(BattleState &state) override;

    [[nodiscard]] std::string getName() const override;
    [[nodiscard]] std::string getDescription() const override;

  private:
    int m_exposureReducedThisBattle{0};

    static constexpr int kExposureOnKill{8};
    static constexpr int kMaxReductionPerBattle{40};
    static constexpr float kExposureGainMultiplier{1.5f};
};