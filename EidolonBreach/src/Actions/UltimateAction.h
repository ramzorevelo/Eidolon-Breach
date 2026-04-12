#pragma once
#include "Actions/IAction.h"

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