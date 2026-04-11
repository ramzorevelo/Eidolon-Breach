#include "StoneGolem.h"

StoneGolem::StoneGolem(std::string name, int maxHp, int maxToughness)
    : Enemy{ std::move(name), maxHp, maxToughness }
{
}

ActionResult StoneGolem::performAttack()
{
    ++m_turnCount;
    if (m_turnCount % 3 == 0)
    {
        ActionResult r{ ActionResult::Type::Damage, 35 };
        r.flavorText = ">> Stone Golem raises both fists -- GROUND SLAM! <<";
        return r;
    }
    return ActionResult{ ActionResult::Type::Damage, 20 };
}
