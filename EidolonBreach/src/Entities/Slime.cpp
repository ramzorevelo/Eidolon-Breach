#include "Entities/Slime.h"
#include "Core/ActionResult.h"

Slime::Slime(std::string name, int maxHp, int maxToughness)
    : Enemy{ std::move(name), maxHp, maxToughness }
{
}

ActionResult Slime::performAttack()
{
    ++m_turnCount;
    if (m_turnCount % 4 == 0)
    {
        ActionResult r{ ActionResult::Type::Heal, 20 };
        r.flavorText = ">> Slime absorbs the moisture -- REGENERATE! <<";
        return r;
    }
    return ActionResult{ ActionResult::Type::Damage, 12 };
}