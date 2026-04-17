#include "Entities/Enemy.h"
#include "Entities/Party.h"
#include "Core/ActionResult.h"
#include "Core/CombatUtils.h"
#include <algorithm>
#include <utility>

Enemy::Enemy(std::string id,
    std::string name,
    Stats stats,
    Affinity affinity,
    int maxToughness,
    std::unique_ptr<IAIStrategy> aiStrategy,
    std::map<Affinity, float> affinityModifiers,
    std::optional<Drop> drop)
    : Unit{std::move(id), std::move(name), stats, affinity}
    , m_toughness{ maxToughness }
    , m_maxToughness{ maxToughness }
    , m_aiStrategy{ std::move(aiStrategy) }
    , m_affinityModifiers{ std::move(affinityModifiers) }
    , m_drop{ std::move(drop) }
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

bool Enemy::hasDrop() const { return m_drop.has_value(); }
const std::optional<Drop>& Enemy::getDrop() const { return m_drop; }

std::optional<Drop> Enemy::dropLoot()
{
    std::optional<Drop> result{ m_drop };
    m_drop = std::nullopt;
    return result;
}

ActionResult Enemy::takeTurn(Party& /*allies*/, Party& targets)
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
            int mitigated{CombatUtils::calculateDamage(result.value, target->getBaseStats().def)};
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