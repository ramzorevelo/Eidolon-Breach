#include "Entities/StoneGolem.h"
#include "Core/ActionResult.h"
#include "Entities/IAIStrategy.h"
#include <memory>

namespace
{
constexpr int kGolemHeavySlamInterval{3};
constexpr int kGolemHeavySlamDamage{35};
constexpr int kGolemBaseDamage{20};
} // namespace

StoneGolem::StoneGolem(std::string name, int maxHp, int maxToughness)
    : Enemy{name + "_golem",
            name,
            Stats{maxHp, maxHp, 15, 20, 5},
            Affinity::Terra,
            maxToughness,
            std::make_unique<BasicAIStrategy>()}
{
    addDrop(Drop{Drop::Type::Gold, 20, {}, 1.0f});
}

ActionResult StoneGolem::performAttack()
{
    ++m_turnCount;
    if (m_turnCount % kGolemHeavySlamInterval == 0)
    {
        ActionResult r{ActionResult::Type::Damage, kGolemHeavySlamDamage};
        r.flavorText = ">> Stone Golem raises both fists -- GROUND SLAM! <<";
        return r;
    }
    return ActionResult{ActionResult::Type::Damage, kGolemBaseDamage};
}