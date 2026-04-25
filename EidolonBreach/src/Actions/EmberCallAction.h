#pragma once
/**
 * @file EmberCallAction.h
 * @brief Ember Call — Lyra's Slot Skill that summons Ignis onto the battlefield.
 *        SP cost: 25. Sets result.summonEffect so Battle spawns the Manifestation.
 *        Only one Slot Skill may be equipped per slot; equip via CharacterSelectScreen.
 */

#include "Actions/IAction.h"
#include "Summons/Ignis.h"

class EmberCallAction : public IAction
{
  public:
    EmberCallAction();

    [[nodiscard]] std::string label() const override;
    [[nodiscard]] bool isAvailable(const PlayableCharacter &user,
                                   const Party &party) const override;
    ActionResult execute(PlayableCharacter &user,
                         Party &allies,
                         Party &enemies,
                         std::optional<TargetInfo> target) override;
    [[nodiscard]] Affinity getAffinity() const override;
    [[nodiscard]] const ActionData &getActionData() const override;

  private:
    ActionData m_data;
    static constexpr int kSpCost{25};
};