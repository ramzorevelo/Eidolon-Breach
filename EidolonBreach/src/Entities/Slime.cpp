#include "Entities/Slime.h"
#include "Battle/BattleState.h"
#include "Core/ActionResult.h"
#include "Entities/IAIStrategy.h"
#include "Entities/Party.h"
#include <memory>

namespace
{
constexpr int kSlimeRegenInterval{4};
constexpr int kSlimeRegenAmount{20};
constexpr int kSlimeBaseDamage{12};
constexpr int kSlimeSplitImpactDamage{8}; 
} // namespace

Slime::Slime(std::string name, int maxHp, int maxToughness)
    : Enemy{name + "_slime",
            name,
            Stats{maxHp, maxHp, 10, 5, 8},
            Affinity::Terra,
            maxToughness,
            std::make_unique<BasicAIStrategy>(),
            {{Affinity::Tempest, 2.0f}, {Affinity::Frost, 0.5f}}}
{
    addDrop(Drop{Drop::Type::Gold, 5, {}, 1.0f});

    setBreakEffect(BreakEffect{
        "slime_split", "Split",
        [](Enemy & /*broken*/, BattleState &state)
        {
            if (state.playerParty == nullptr)
                return;
            for (Unit *u : state.playerParty->getAliveUnits())
                u->takeTrueDamage(kSlimeSplitImpactDamage);
        }});
}

ActionResult Slime::performAttack()
{
    ++m_turnCount;
    if (m_turnCount % kSlimeRegenInterval == 0)
    {
        ActionResult r{ActionResult::Type::Heal, kSlimeRegenAmount};
        r.flavorText = ">> " + getName() + " absorbs the moisture -- REGENERATE! <<";
        return r;
    }
    return ActionResult{ActionResult::Type::Damage, kSlimeBaseDamage};
}

std::string Slime::getIntentLabel() const
{
    if (isBroken())
        return "";
    // Next turn's turn count will be m_turnCount + 1.
    if ((m_turnCount + 1) % kSlimeRegenInterval == 0)
        return "Regenerate";
    return "Attack";
}