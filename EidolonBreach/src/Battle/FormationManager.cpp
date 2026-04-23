/**
 * @file FormationManager.cpp
 * @brief FormationManager implementation.
 */

#include "Battle/FormationManager.h"
#include "Entities/Unit.h"
#include <algorithm>

bool FormationManager::insert(Unit *unit, int position)
{
    if (!unit || m_count >= kMaxSlots || position < 0 || position >= kMaxSlots)
        return false;

    // Shift occupants at or after position toward the rear.
    for (int i{m_count}; i > position; --i)
        m_slots[static_cast<std::size_t>(i)] =
            m_slots[static_cast<std::size_t>(i - 1)];

    m_slots[static_cast<std::size_t>(position)] = unit;
    ++m_count;
    return true;
}

void FormationManager::remove(Unit *unit)
{
    const int pos{getPosition(unit)};
    if (pos == -1)
        return;
    shiftForward(pos);
    --m_count;
}

bool FormationManager::swap(Unit *unit, int targetPosition)
{
    const int srcPos{getPosition(unit)};
    if (srcPos == -1 || targetPosition < 0 || targetPosition >= kMaxSlots)
        return false;

    std::swap(m_slots[static_cast<std::size_t>(srcPos)],
              m_slots[static_cast<std::size_t>(targetPosition)]);
    return true;
}

int FormationManager::applyPush(Unit *unit,
                                int delta,
                                float skillPower,
                                ScalingStat scaling,
                                const Stats &attackerFinal)
{
    const int pos{getPosition(unit)};
    if (pos == -1)
        return 0;

    const int target{pos + delta};
    const int clamped{std::clamp(target, 0, kMaxSlots - 1)};

    if (clamped != pos)
        swap(unit, clamped);

    if (target != clamped)
        return CombatUtils::calculateDamage(skillPower, attackerFinal, attackerFinal, scaling);

    return 0;
}

Unit *FormationManager::getAt(int position) const
{
    if (position < 0 || position >= kMaxSlots)
        return nullptr;
    return m_slots[static_cast<std::size_t>(position)];
}

int FormationManager::getPosition(const Unit *unit) const
{
    for (int i{0}; i < kMaxSlots; ++i)
        if (m_slots[static_cast<std::size_t>(i)] == unit)
            return i;
    return -1;
}

int FormationManager::getCount() const
{
    return m_count;
}

void FormationManager::shiftForward(int fromPosition)
{
    for (int i{fromPosition}; i < m_count - 1; ++i)
        m_slots[static_cast<std::size_t>(i)] =
            m_slots[static_cast<std::size_t>(i + 1)];
    m_slots[static_cast<std::size_t>(m_count - 1)] = nullptr;
}

void FormationManager::shiftBackward(int fromPosition)
{
    for (int i{m_count}; i > fromPosition; --i)
        m_slots[static_cast<std::size_t>(i)] =
            m_slots[static_cast<std::size_t>(i - 1)];
    m_slots[static_cast<std::size_t>(fromPosition)] = nullptr;
}