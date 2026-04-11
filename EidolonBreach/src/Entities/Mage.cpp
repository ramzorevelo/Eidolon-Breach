#include "Entities/Mage.h"
#include "Core/ActionResult.h"
#include <string>

Mage::Mage(std::string name)
    : Player{ std::move(name), 90 }
{
}

ActionResult Mage::useSkill()
{
    // Apply SP and energy side effects from base class
    Player::useSkill();
    // Mage deals 40 damage instead of the base 28
    return ActionResult{ ActionResult::Type::Damage, 40 };
}