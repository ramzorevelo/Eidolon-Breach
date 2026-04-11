#pragma once
#include "Actions/IAction.h"

class BasicStrikeAction : public IAction
{
public:
    std::string label() const override;
    ActionResult execute(Player& player, Enemy& enemy) override;
};