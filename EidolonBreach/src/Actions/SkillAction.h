#pragma once
#include "Actions/IAction.h"

/**
 * @file SkillAction.h
 * @brief A skill that consumes SP to deal damage.
 */

/** Skill action: consumes 1 SP, grants 30 Energy, deals configurable damage. */
class SkillAction : public IAction
{
public:
    explicit SkillAction(int damage = 28);

    std::string  label()                              const override;
    bool         isAvailable(const PlayableCharacter&) const override;
    ActionResult execute(PlayableCharacter& user,
        Party& allies,
        Party& enemies,
        std::optional<TargetInfo> target) override;

private:
    int m_damage;
};