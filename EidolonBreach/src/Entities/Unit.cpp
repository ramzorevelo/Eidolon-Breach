#include "Entities/Unit.h"
#include "Core/ActionResult.h"
#include <algorithm>
#include <utility>

/**
 * @file Unit.cpp
 * @brief Implementation of the abstract Unit class.
 */
Unit::Unit(std::string id,
           std::string name,
           Stats stats,
           Affinity affinity)
    : m_id{std::move(id)}, m_name{std::move(name)}, m_stats{stats}, m_affinity{affinity}
{
}

const std::string& Unit::getId()                    const { return m_id; }
const std::string& Unit::getName()                  const { return m_name; }
Affinity           Unit::getAffinity()              const { return m_affinity; }
const Stats &Unit::getBaseStats() const
{
    return m_stats;
}

Stats Unit::getFinalStats() const
{
    Stats result{getBaseStats()};
    for (const auto &effect : m_effects) // pass 1: flat additions
        result = effect->modifyStatsFlat(result);
    for (const auto &effect : m_effects) // pass 2: percentage multipliers
        result = effect->modifyStatsPct(result);
    return result;
}
int                Unit::getHp()                    const { return m_stats.hp; }
int                Unit::getMaxHp()                 const { return m_stats.maxHp; }
bool               Unit::isAlive()                  const { return m_stats.hp > 0; }

void Unit::takeDamage(int amount)
{
    int remaining{std::max(0, amount)};

    // Run the shield absorption pass.
    for (auto &effect : m_effects)
    {
        if (remaining <= 0)
            break;
        remaining = effect->absorbDamage(remaining);
    }
    m_stats.hp = std::max(0, m_stats.hp - remaining);
}

void Unit::takeTrueDamage(int amount)
{
    // Bypasses shield absorption — used by DoT effects (see BurnEffect::onTick).
    m_stats.hp = std::max(0, m_stats.hp - amount);
}

void Unit::applyEffect(std::unique_ptr<IStatusEffect> effect)
{
    // Refresh semantics: replace existing effect with the same ID.
    for (auto &existing : m_effects)
    {
        if (existing->getId() == effect->getId())
        {
            existing = std::move(effect);
            existing->onApply(*this);
            return;
        }
    }
    effect->onApply(*this);
    m_effects.push_back(std::move(effect));
}

void Unit::removeEffect(std::string_view id)
{
    auto it = std::find_if(m_effects.begin(), m_effects.end(),
                           [id](const auto &e)
                           { return e->getId() == id; });
    if (it != m_effects.end())
    {
        (*it)->onRemove(*this);
        m_effects.erase(it);
    }
}

void Unit::removeEffectsByTag(std::string_view tag)
{
    // Call onRemove before erasing.
    for (auto &effect : m_effects)
        if (effect->hasTag(tag))
            effect->onRemove(*this);

    m_effects.erase(
        std::remove_if(m_effects.begin(), m_effects.end(),
                       [tag](const auto &e)
                       { return e->hasTag(tag); }),
        m_effects.end());
}

void Unit::extendEffectsByTag(std::string_view tag, int turns)
{
    for (auto &effect : m_effects)
        if (effect->hasTag(tag))
            effect->extendDuration(turns);
}

bool Unit::hasEffect(std::string_view id) const
{
    for (const auto &effect : m_effects)
        if (effect->getId() == id)
            return true;
    return false;
}

bool Unit::hasEffectWithTag(std::string_view tag) const
{
    for (const auto &effect : m_effects)
        if (effect->hasTag(tag))
            return true;
    return false;
}

std::vector<std::string> Unit::tickEffects()
{
    // Snapshot raw pointers BEFORE ticking because onTick may exhaust shields,
    // which would not invalidate m_effects but we still want stable iteration.
    std::vector<IStatusEffect *> snapshot{};
    for (auto &effect : m_effects)
        snapshot.push_back(effect.get());

    std::vector<std::string> messages{};
    for (IStatusEffect *effect : snapshot)
    {
        std::string msg{effect->onTick(*this)};
        if (!msg.empty())
            messages.push_back(std::move(msg));

        // Decrement duration for timed effects. Permanent effects (nullopt) are unaffected.
        effect->extendDuration(-1);
    }

    // Remove exhausted shields and zero-duration effects.
    m_effects.erase(
        std::remove_if(m_effects.begin(), m_effects.end(),
                       [](const auto &e)
                       {
                           return e->isExhausted() ||
                                  (e->getDuration().has_value() && *e->getDuration() <= 0);
                       }),
        m_effects.end());

    return messages;
}

const std::vector<std::unique_ptr<IStatusEffect>> &Unit::getEffects() const
{
    return m_effects;
}

void Unit::heal(int amount)
{
    m_stats.hp = std::min(m_stats.maxHp, m_stats.hp + amount);
}