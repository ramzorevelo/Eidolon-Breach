#pragma once

/**
 * @file SkillAction.h
 * @brief A skill that consumes SP to deal damage.
 */

#include "Actions/IAction.h"

/** Skill action: consumes 25 SP, grants +15 Energy, deals configurable damage. */
class SkillAction : public IAction
{
  public:
    explicit SkillAction(int damage = 28);

    std::string label() const override;
    bool isAvailable(const PlayableCharacter &user, const Party &party) const override;
    ActionResult execute(PlayableCharacter &user,
                         Party &allies,
                         Party &enemies,
                         std::optional<TargetInfo> target) override;
    Affinity getAffinity() const override
    {
        return Affinity::Blaze;
    } // placeholder

  private:
    int m_damage;
    static constexpr int kSpCost{25};
    static constexpr int kEnergyGain{15};
};