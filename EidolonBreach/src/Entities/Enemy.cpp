#include "Entities/Enemy.h"
#include "Entities/Party.h"
#include "Core/ActionResult.h"
#include "Core/CombatUtils.h"
#include <algorithm>
#include <utility>
#include <random>

Enemy::Enemy(std::string id,
    std::string name,
    Stats stats,
    Affinity affinity,
    int maxToughness,
    std::unique_ptr<IAIStrategy> aiStrategy,
    std::map<Affinity, float> affinityModifiers)
    : Unit{std::move(id), std::move(name), stats, affinity}
    , m_toughness{ maxToughness }
    , m_maxToughness{ maxToughness }
    , m_aiStrategy{ std::move(aiStrategy) }
    , m_affinityModifiers{ std::move(affinityModifiers) }
{
}

bool Enemy::isBroken() const { return m_isBroken; }
int Enemy::getToughness() const { return m_toughness; }
int Enemy::getMaxToughness() const { return m_maxToughness; }

void Enemy::applyToughnessHit(int amount)
{
    m_toughness = std::max(0, m_toughness - amount);
    if (m_toughness == 0)
    {
        m_isBroken = true;
        m_toughness = m_maxToughness;
    }
}

void Enemy::recoverFromBreak()
{
    m_isBroken = false;
}

float Enemy::getAffinityModifier(Affinity a) const
{
    auto it = m_affinityModifiers.find(a);
    return (it != m_affinityModifiers.end()) ? it->second : 1.0f;
}

void Enemy::addDrop(Drop drop)
{
    m_dropPool.push_back(std::move(drop));
}

std::vector<Drop> Enemy::generateDrops(unsigned int seed) const
{
    std::mt19937 rng{seed};
    std::uniform_real_distribution<float> dist{0.0f, 1.0f};

    std::vector<Drop> result{};
    for (const Drop &drop : m_dropPool)
    {
        if (drop.type == Drop::Type::GuaranteedItem || dist(rng) <= drop.dropChance)
            result.push_back(drop);
    }
    return result;
}

ActionResult Enemy::takeTurn(Party & /*allies*/, Party &targets, BattleState & /*state*/)
{
    if (m_isBroken)
    {
        recoverFromBreak();
        return ActionResult{ ActionResult::Type::Skip, 0 };
    }

    AIDecision decision = m_aiStrategy->decide(*this, targets);
    Unit* target = targets.getUnitAt(decision.targetIndex);

    ActionResult result = performAttack();

    if (target && target->isAlive())
    {
        switch (result.type)
        {
        case ActionResult::Type::Damage:
        {
            int mitigated{CombatUtils::calculateDamage(result.value, target->getFinalStats().def)};
            target->takeDamage(mitigated);
            result.value = mitigated; // keep result accurate for UI rendering
            break;
        }
        case ActionResult::Type::Heal:
            heal(result.value);
            break;
        default:
            break;
        }
    }
    return result;
}

ActionResult Enemy::performAttack()
{
    return ActionResult{ ActionResult::Type::Damage, 20 };
}