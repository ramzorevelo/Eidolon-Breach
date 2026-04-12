#include "Entities/VampireBat.h"
#include "Core/ActionResult.h"

VampireBat::VampireBat(std::string name, int maxHp, int maxToughness)
    : Enemy{ name + "_bat",
             name,
             Stats{ maxHp, maxHp, 14, 8, 14 },
             Affinity::Blaze,
             maxToughness }
{
}

ActionResult VampireBat::performAttack()
{
    ++m_turnCount;
    if (m_turnCount % 3 == 0)
    {
        heal(12);   // lifedrain – heals self directly, still deals damage to player
        ActionResult r{ ActionResult::Type::Damage, 8 };
        r.flavorText = ">> Vampire Bat drains your life force! <<";
        return r;
    }
    return ActionResult{ ActionResult::Type::Damage, 14 };
}