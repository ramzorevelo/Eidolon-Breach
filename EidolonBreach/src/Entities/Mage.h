#pragma once

#include "Entities/Player.h"

class Mage : public Player
{
public:
    explicit Mage(std::string name);
 
    ActionResult useSkill() override;
};
