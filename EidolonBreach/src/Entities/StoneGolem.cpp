#include "Entities/StoneGolem.h"

StoneGolem::StoneGolem(std::string name, int maxHp, int maxToughness)
    : Enemy{ name + "_golem",
             name,
             Stats{ maxHp, maxHp, 15, 20, 5 },
             Affinity::Terra,
             maxToughness }
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