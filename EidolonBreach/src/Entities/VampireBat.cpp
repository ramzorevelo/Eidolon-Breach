#include "Entities/VampireBat.h"
#include "Entities/IAIStrategy.h"
#include "Core/ActionResult.h"
#include <memory>

namespace {
    constexpr int kBatLifedrainInterval{ 3 };
    constexpr int kBatLifedrainHeal{ 12 };
    constexpr int kBatLifedrainDamage{ 8 };
    constexpr int kBatBaseDamage{ 14 };
}

VampireBat::VampireBat(std::string name, int maxHp, int maxToughness)
    : Enemy{name + "_bat",
            name,
            Stats{maxHp, maxHp, 14, 8, 14},
            Affinity::Blaze,
            maxToughness,
            std::make_unique<BasicAIStrategy>()}
{
    addDrop(Drop{Drop::Type::Gold, 10, {}, 1.0f});
}

ActionResult VampireBat::performAttack()
{
    ++m_turnCount;
    if (m_turnCount % kBatLifedrainInterval == 0)
    {
        heal(kBatLifedrainHeal);
        ActionResult r{ ActionResult::Type::Damage, kBatLifedrainDamage };
        r.flavorText = ">> Vampire Bat drains your life force! <<";
        return r;
    }
    return ActionResult{ ActionResult::Type::Damage, kBatBaseDamage };
}