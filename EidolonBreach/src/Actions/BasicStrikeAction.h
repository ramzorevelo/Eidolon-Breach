#pragma once
#include "Actions/IAction.h"

class BasicStrikeAction : public IAction
{
public:
    std::string  label()       const override;
    ActionResult execute(PlayableCharacter& user,
        Party& allies,
        Party& enemies,
        std::optional<TargetInfo> target) override;
};