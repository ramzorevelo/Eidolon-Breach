#pragma once
#include "Actions/IAction.h"

/**
 * @file UltimateAction.h
 * @brief Ultimate ability that consumes all Energy for massive damage.
 */

/** Ultimate action: requires full Energy, resets it, grants +2 SP. */
class UltimateAction : public IAction
{
public:
    std::string  label()                              const override;
    bool         isAvailable(const PlayableCharacter&) const override;
    ActionResult execute(PlayableCharacter& user,
        Party& allies,
        Party& enemies,
        std::optional<TargetInfo> target) override;
};