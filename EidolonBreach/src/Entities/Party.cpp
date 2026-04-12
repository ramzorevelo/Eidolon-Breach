#include "Entities/Party.h"
#include <algorithm>

void Party::addUnit(std::unique_ptr<Unit> unit)
{
    m_units.push_back(std::move(unit));
}

bool Party::isAllDead() const
{
    for (const auto& u : m_units)
        if (u->isAlive()) return false;
    return true;
}

std::vector<Unit*> Party::getAliveUnits() const
{
    std::vector<Unit*> result;
    for (const auto& u : m_units)
        if (u->isAlive()) result.push_back(u.get());
    return result;
}

Unit* Party::getUnitAt(std::size_t index)
{
    return (index < m_units.size()) ? m_units[index].get() : nullptr;
}

const Unit* Party::getUnitAt(std::size_t index) const
{
    return (index < m_units.size()) ? m_units[index].get() : nullptr;
}

std::size_t Party::size() const { return m_units.size(); }

bool Party::contains(const Unit* unit) const
{
    for (const auto& u : m_units)
        if (u.get() == unit) return true;
    return false;
}

std::size_t Party::getIndex(const Unit* unit) const
{
    for (std::size_t i = 0; i < m_units.size(); ++i)
        if (m_units[i].get() == unit) return i;
    return m_units.size();   // sentinel: not found
}