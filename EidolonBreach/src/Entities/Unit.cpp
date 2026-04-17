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
const Stats& Unit::getStats()                       const { return m_stats; }
int                Unit::getHp()                    const { return m_stats.hp; }
int                Unit::getMaxHp()                 const { return m_stats.maxHp; }
bool               Unit::isAlive()                  const { return m_stats.hp > 0; }

void Unit::takeDamage(int amount)
{
    m_stats.hp = std::max(0, m_stats.hp - amount);
}

void Unit::heal(int amount)
{
    m_stats.hp = std::min(m_stats.maxHp, m_stats.hp + amount);
}