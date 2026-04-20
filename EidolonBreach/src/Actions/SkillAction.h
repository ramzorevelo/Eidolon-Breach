#pragma once


#include "Actions/IAction.h"


class SkillAction : public IAction
{
  public:
    /** @param skillPower Damage coefficient applied to user ATK (default 2.0). */
    explicit SkillAction(float skillPower = 2.0f);

    std::string label() const override;
    bool isAvailable(const PlayableCharacter &user,
                     const Party &party) const override;
    ActionResult execute(PlayableCharacter &user,
                         Party &allies,
                         Party &enemies,
                         std::optional<TargetInfo> target) override;
    Affinity getAffinity() const override;
    const ActionData &getActionData() const override;

  private:
    ActionData m_data;
};