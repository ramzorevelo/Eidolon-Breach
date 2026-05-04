#include "Entities/Enemy.h"
#include "Core/ActionResult.h"
#include "Core/CombatUtils.h"
#include "Entities/Party.h"
#include <algorithm>
#include <random>
#include <utility>

Enemy::Enemy(std::string id,
             std::string name,
             Stats stats,
             Affinity affinity,
             int maxToughness,
             std::unique_ptr<IAIStrategy> aiStrategy,
             std::map<Affinity, float> affinityModifiers)
    : Unit{std::move(id), std::move(name), stats, affinity}, m_toughness{maxToughness}, m_maxToughness{maxToughness}, m_aiStrategy{std::move(aiStrategy)}, m_affinityModifiers{std::move(affinityModifiers)}
{
}

bool Enemy::isBroken() const
{
    return m_isBroken;
}

int Enemy::getToughness() const
{
    return m_toughness;
}
int Enemy::getMaxToughness() const
{
    return m_maxToughness;
}

void Enemy::applyToughnessHit(int amount, Affinity sourceAffinity)
{
    int scaled{static_cast<int>(static_cast<float>(amount) * getAffinityModifier(sourceAffinity))};
    m_toughness = std::max(0, m_toughness - scaled);
    if (m_toughness == 0)
    {
        m_isBroken = true;
        m_brokenTurnsRemaining = 1;
        m_toughness = m_maxToughness; // reset immediately per GDD §4.10
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

float Enemy::getToughnessAffinityModifier(Affinity affinity) const
{
    // Delegates to the existing affinity modifier map — no separate data needed.
    return getAffinityModifier(affinity);
}

void Enemy::setBreakEffect(BreakEffect effect)
{
    m_breakEffect = std::move(effect);
}

const BreakEffect &Enemy::getBreakEffect() const
{
    return m_breakEffect;
}

float Enemy::getBrokenDamageBonus() const
{
    return m_brokenDamageBonus;
}

void Enemy::takeDamage(int amount)
{
    // Apply broken damage bonus after DEF reduction (caller already reduced by DEF).
    int effective{m_isBroken
                      ? static_cast<int>(static_cast<float>(amount) * m_brokenDamageBonus)
                      : amount};
    Unit::takeDamage(effective);
}

void Enemy::takeTrueDamage(int amount)
{
    // DoT during the break window also benefits from the bonus (GDD §4.10).
    int effective{m_isBroken
                      ? static_cast<int>(static_cast<float>(amount) * m_brokenDamageBonus)
                      : amount};
    Unit::takeTrueDamage(effective);
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

void Enemy::scaleMaxToughness(float factor)
{
    m_maxToughness = std::max(1, static_cast<int>(static_cast<float>(m_maxToughness) * factor));
    m_toughness = m_maxToughness;
}

ActionResult Enemy::takeTurn(Party & /*allies*/, Party &targets, BattleState & /*state*/)
{
    if (m_isBroken)
    {
        // Decrement the break window. When it reaches 0 the bonus ends and the
        // enemy is no longer considered broken.
        --m_brokenTurnsRemaining;
        if (m_brokenTurnsRemaining <= 0)
            m_isBroken = false;
        return ActionResult{ActionResult::Type::Skip, 0};
    }

    AIDecision decision = m_aiStrategy->decide(*this, targets);
    Unit *target = targets.getUnitAt(decision.targetIndex);

    ActionResult result = performAttack();

    if (target && target->isAlive())
    {
        switch (result.type)
        {
        case ActionResult::Type::Damage:
        {
            int mitigated{CombatUtils::calculateDamage(result.value, target->getFinalStats().def)};
            target->takeDamage(mitigated);
            result.value = mitigated;
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
    return ActionResult{ActionResult::Type::Damage, 20};
}

std::string Enemy::getIntentLabel() const
{
    if (m_isBroken)
        return "Stunned (skip)";
    return "Attacks";
}