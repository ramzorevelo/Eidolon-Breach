#include "Entities/Slime.h"
#include "Core/ActionResult.h"
#include "Entities/IAIStrategy.h"
#include <memory>

namespace
{
constexpr int kSlimeRegenInterval{4};
constexpr int kSlimeRegenAmount{20};
constexpr int kSlimeBaseDamage{12};
} // namespace

Slime::Slime(std::string name, int maxHp, int maxToughness)
    : Enemy{name + "_slime",
            name,
            Stats{maxHp, maxHp, 10, 5, 8},
            Affinity::Terra,
            maxToughness,
            std::make_unique<BasicAIStrategy>()}
{
}

ActionResult Slime::performAttack()
{
    ++m_turnCount;
    if (m_turnCount % kSlimeRegenInterval == 0)
    {
        ActionResult r{ActionResult::Type::Heal, kSlimeRegenAmount};
        r.flavorText = ">> Slime absorbs the moisture -- REGENERATE! <<";
        return r;
    }
    return ActionResult{ActionResult::Type::Damage, kSlimeBaseDamage};
}