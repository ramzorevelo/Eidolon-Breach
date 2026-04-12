#include "Entities/Enemy.h"
#include "Entities/Party.h"
#include "Core/ActionResult.h"
#include <algorithm>
#include <utility>

Enemy::Enemy(std::string               id,
    std::string               name,
    Stats                     stats,
    Affinity                  affinity,
    int                       maxToughness,
    std::map<Affinity, float> affinityModifiers,
    std::optional<Drop>       drop)
    : Unit{ std::move(id), std::move(name), stats, affinity, 0 }
    , m_toughness{ maxToughness }
    , m_maxToughness{ maxToughness }
    , m_affinityModifiers{ std::move(affinityModifiers) }
    , m_drop{ std::move(drop) }
{
}

// ── Toughness ────────────────────────────────────────────────────────────
bool Enemy::isBroken()        const { return m_isBroken; }
int  Enemy::getToughness()    const { return m_toughness; }
int  Enemy::getMaxToughness() const { return m_maxToughness; }

void Enemy::applyToughnessHit(int amount)
{
    // Phase 4 will multiply by m_affinityModifiers here.
    // For now, apply damage at face value.
    m_toughness = std::max(0, m_toughness - amount);
    if (m_toughness == 0)
    {
        m_isBroken = true;
        m_toughness = m_maxToughness;   // reset gauge for next cycle
    }
}

void Enemy::recoverFromBreak()
{
    m_isBroken = false;
}

// ── Affinity modifiers ───────────────────────────────────────────────────
float Enemy::getAffinityModifier(Affinity a) const
{
    auto it = m_affinityModifiers.find(a);
    return (it != m_affinityModifiers.end()) ? it->second : 1.0f;
}

// ── Drops ────────────────────────────────────────────────────────────────
bool                       Enemy::hasDrop()  const { return m_drop.has_value(); }
const std::optional<Drop>& Enemy::getDrop()  const { return m_drop; }

std::optional<Drop> Enemy::dropLoot()
{
    std::optional<Drop> result{ m_drop };
    m_drop = std::nullopt;
    return result;
}

// ── Turn ─────────────────────────────────────────────────────────────────
ActionResult Enemy::takeTurn(Party& /*allies*/, Party& enemies)
{
    if (m_isBroken)
    {
        recoverFromBreak();
        return ActionResult{ ActionResult::Type::Skip, 0 };
    }

    // Simple AI: attack the first alive target in the player party.
    Unit* target = nullptr;
    for (std::size_t i = 0; i < enemies.size(); ++i)
    {
        Unit* u = enemies.getUnitAt(i);
        if (u && u->isAlive()) { target = u; break; }
    }

    ActionResult result = performAttack();

    if (target)
    {
        switch (result.type)
        {
        case ActionResult::Type::Damage:
            target->takeDamage(result.value);
            break;
        case ActionResult::Type::Heal:
            heal(result.value);      // enemy heals itself
            break;
        default:
            break;
        }
    }
    return result;
}

// ── Default attack ───────────────────────────────────────────────────────
ActionResult Enemy::performAttack()
{
    return ActionResult{ ActionResult::Type::Damage, 20 };
}