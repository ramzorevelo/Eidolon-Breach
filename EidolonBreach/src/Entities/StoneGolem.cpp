#include "Entities/StoneGolem.h"
#include "Battle/BattleState.h"
#include "Core/ActionResult.h"
#include "Core/Effects/ShieldEffect.h"
#include "Entities/IAIStrategy.h"
#include "Entities/Party.h"
#include <memory>

namespace
{
constexpr int kGolemHeavySlamInterval{3};
constexpr int kGolemHeavySlamDamage{35};
constexpr int kGolemBaseDamage{20};
constexpr int kCrumbleShieldAmount{25};
constexpr int kCrumbleShieldDuration{3};
} // namespace

StoneGolem::StoneGolem(std::string name, int maxHp, int maxToughness)
    : Enemy{name + "_golem",
            name,
            Stats{maxHp, maxHp, 15, 20, 5},
            Affinity::Terra,
            maxToughness,
            std::make_unique<BasicAIStrategy>(),
            {{Affinity::Tempest, 2.0f}, {Affinity::Frost, 0.5f}}}
{
    addDrop(Drop{Drop::Type::Gold, 20, {}, 1.0f});

    // Crumble: applies a shield to all player allies.
    // Full "DEF +15% for 3 rounds" requires a percentage-buff IStatusEffect (Phase 9).
    setBreakEffect(BreakEffect{
        "golem_crumble", "Crumble",
        [](Enemy & /*broken*/, BattleState &state)
        {
            if (state.playerParty == nullptr)
                return;
            for (Unit *u : state.playerParty->getAliveUnits())
                u->applyEffect(std::make_unique<ShieldEffect>(kCrumbleShieldAmount,
                                                              kCrumbleShieldDuration));
        }});
}

ActionResult StoneGolem::performAttack()
{
    ++m_turnCount;
    if (m_turnCount % kGolemHeavySlamInterval == 0)
    {
        ActionResult r{ActionResult::Type::Damage, kGolemHeavySlamDamage};
        r.flavorText = ">> " + getName() + " raises both fists -- GROUND SLAM! <<";
        return r;
    }
    return ActionResult{ActionResult::Type::Damage, kGolemBaseDamage};
}


std::string StoneGolem::getIntentLabel() const
{
    if (isBroken())
        return "";
    if ((m_turnCount + 1) % kGolemHeavySlamInterval == 0)
        return "use Heavy Slam";
    return "Attacks";
}