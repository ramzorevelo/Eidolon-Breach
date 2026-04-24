#include "Entities/VampireBat.h"
#include "Battle/BattleState.h"
#include "Core/ActionResult.h"
#include "Entities/IAIStrategy.h"
#include "Entities/Party.h"
#include <memory>

namespace
{
constexpr int kBatLifedrainInterval{3};
constexpr int kBatLifedrainHeal{12};
constexpr int kBatLifedrainDamage{8};
constexpr int kBatBaseDamage{14};
} // namespace

VampireBat::VampireBat(std::string name, int maxHp, int maxToughness)
    : Enemy{name + "_bat",
            name,
            Stats{maxHp, maxHp, 14, 8, 14},
            Affinity::Blaze,
            maxToughness,
            std::make_unique<BasicAIStrategy>(),
            {{Affinity::Terra, 2.0f}, {Affinity::Tempest, 0.5f}}}
{
    addDrop(Drop{Drop::Type::Gold, 10, {}, 1.0f});

    // Bloodless: heals all player allies 10% max HP; zeroes bat's damage for 2 turns.
    setBreakEffect(BreakEffect{
        "bat_bloodless", "Bloodless",
        [](Enemy &broken, BattleState &state)
        {
            // Heal each player ally for 10% of their max HP.
            if (state.playerParty != nullptr)
            {
                for (Unit *u : state.playerParty->getAliveUnits())
                    u->heal(u->getMaxHp() / 10);
            }
            // Activate bloodless on the bat via virtual dispatch — no cast needed.
            broken.onBreakCallback();
        }});
}

void VampireBat::activateBloodless()
{
    m_bloodlessActive = true;
    m_bloodlessTurnsRemaining = kBloodlessDuration;
}

void VampireBat::onBreakCallback()
{
    activateBloodless();
}

ActionResult VampireBat::performAttack()
{
    if (m_bloodlessActive)
    {
        --m_bloodlessTurnsRemaining;
        if (m_bloodlessTurnsRemaining <= 0)
            m_bloodlessActive = false;
        ActionResult r{ActionResult::Type::Skip, 0};
        r.flavorText = ">> Vampire Bat is bloodless — no damage this turn! <<";
        return r;
    }

    ++m_turnCount;
    if (m_turnCount % kBatLifedrainInterval == 0)
    {
        heal(kBatLifedrainHeal);
        ActionResult r{ActionResult::Type::Damage, kBatLifedrainDamage};
        r.flavorText = ">> " + getName() + " drains your life force! <<";
        return r;
    }
    return ActionResult{ActionResult::Type::Damage, kBatBaseDamage};
}