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
constexpr int kSlimeSplitImpactDamage{8}; ///< Simulates fragment impact until Phase 6 spawn.
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

    // Split: deals impact damage to all player units (stub).
    // Full behaviour (spawning two Slime Fragments) requires FormationManager (Phase 6).
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