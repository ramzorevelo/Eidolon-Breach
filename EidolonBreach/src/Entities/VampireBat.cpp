#include "Entities/VampireBat.h"
#include "Core/ActionResult.h"

VampireBat::VampireBat(std::string name, int maxHp, int maxToughness)
    : Enemy{ std::move(name), maxHp, maxToughness }
{
}

ActionResult VampireBat::performAttack()
{
    ++m_turnCount;
    if (m_turnCount % 3 == 0)
    {
        heal(12);
        ActionResult r{ ActionResult::Type::Damage, 8 };
        r.flavorText = ">> Vampire Bat drains your life force! <<";
        return r;
    }
    return ActionResult{ ActionResult::Type::Damage, 14 };
}