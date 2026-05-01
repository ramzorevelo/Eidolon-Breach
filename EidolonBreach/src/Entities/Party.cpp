#include "Entities/Party.h"
#include <algorithm>

void Party::addUnit(std::unique_ptr<Unit> unit) {
	m_units.push_back(std::move(unit));
}

bool Party::isAllDead() const {
	for (const auto& u : m_units)
		if (u->isAlive()) return false;
	return true;
}

std::vector<Unit*> Party::getAliveUnits() const {
	std::vector<Unit*> result;
	for (const auto& u : m_units)
		if (u->isAlive()) result.push_back(u.get());
	return result;
}

Unit* Party::getUnitAt(std::size_t index) {
	return (index < m_units.size()) ? m_units[index].get() : nullptr;
}

const Unit* Party::getUnitAt(std::size_t index) const {
	return (index < m_units.size()) ? m_units[index].get() : nullptr;
}

std::size_t Party::size() const { return m_units.size(); }

bool Party::contains(const Unit* unit) const {
	for (const auto& u : m_units)
		if (u.get() == unit) return true;
	return false;
}

std::size_t Party::getIndex(const Unit* unit) const {
	for (std::size_t i{ 0 }; i < m_units.size(); ++i)
		if (m_units[i].get() == unit) return i;
	return m_units.size();
}
void Party::removeUnit(std::string_view unitId)
{
    auto it{std::find_if(m_units.begin(), m_units.end(),
                         [unitId](const std::unique_ptr<Unit> &u)
                         { return u && u->getId() == unitId; })};
    if (it != m_units.end())
        m_units.erase(it);
}

void Party::gainSp(int amount) {
	m_resources.sp = std::min(m_resources.maxSp, m_resources.sp + amount);
}

bool Party::useSp(int amount) {
	if (m_resources.sp < amount) return false;
	m_resources.sp -= amount;
	return true;
}

std::optional<std::unique_ptr<IVestige>> Party::addVestige(std::unique_ptr<IVestige> vestige)
{
    if (m_vestiges.size() >= static_cast<std::size_t>(kMaxVestiges))
        return vestige; // return ownership back to caller
    m_vestiges.push_back(std::move(vestige));
    return std::nullopt; // successfully stored
}

bool Party::removeVestige(std::size_t index)
{
    if (index >= m_vestiges.size())
        return false;
    m_vestiges.erase(m_vestiges.begin() + static_cast<std::ptrdiff_t>(index));
    return true;
}
const std::vector<std::unique_ptr<IVestige>> &Party::getVestiges() const
{
    return m_vestiges;
}