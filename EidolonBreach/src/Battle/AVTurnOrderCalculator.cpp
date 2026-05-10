/**
 * @file AVTurnOrderCalculator.cpp
 * @brief AVTurnOrderCalculator implementation.
 */

#include "Battle/AVTurnOrderCalculator.h"
#include "Core/CombatConstants.h"
#include "Entities/Party.h"
#include "Entities/PlayableCharacter.h"
#include "Entities/Unit.h"
#include <algorithm>
#include <limits>

void AVTurnOrderCalculator::syncUnits(Party &playerParty, Party &enemyParty) const
{
    auto registerParty = [this](Party &party)
    {
        for (std::size_t i{0}; i < party.size(); ++i)
        {
            Unit *u{party.getUnitAt(i)};
            if (!u || !u->isAlive())
                continue;
            if (m_baseAV.find(u) == m_baseAV.end())
            {
                const float base{computeBaseAV(u)};
                m_baseAV[u] = base;
                m_currentAV[u] = base;
            }
        }
    };
    registerParty(playerParty);
    registerParty(enemyParty);
}

float AVTurnOrderCalculator::computeBaseAV(const Unit *unit)
{
    const int spd{unit->getFinalStats().spd};
    if (spd <= 0)
        return CombatConstants::kBaseAG;
    return CombatConstants::kBaseAG / static_cast<float>(spd);
}

float AVTurnOrderCalculator::exposureModifier(Unit *unit)
{
    const PlayableCharacter *pc{unit->asPlayableCharacter()};
    if (!pc)
        return CombatConstants::kAvModBaseline;
    return pc->getExposureAVModifier();
}

std::vector<TurnSlot> AVTurnOrderCalculator::calculate(Party &playerParty,
                                                       Party &enemyParty) const
{
    syncUnits(playerParty, enemyParty);

    // Build a working copy from live state, restricted to alive units only.
    std::unordered_map<Unit *, float> workAV{};
    std::vector<std::pair<Unit *, bool>> alive{}; // (unit, isPlayer)

    auto collectAlive = [&](Party &party, bool isPlayer)
    {
        for (std::size_t i{0}; i < party.size(); ++i)
        {
            Unit *u{party.getUnitAt(i)};
            if (!u || !u->isAlive())
                continue;
            auto it{m_currentAV.find(u)};
            workAV[u] = (it != m_currentAV.end()) ? it->second : computeBaseAV(u);
            alive.emplace_back(u, isPlayer);
        }
    };
    collectAlive(playerParty, true);
    collectAlive(enemyParty, false);

    if (alive.empty())
        return {};

    auto partyIndex = [&](Unit *u, bool isPlayer) -> std::size_t
    {
        Party &party{isPlayer ? playerParty : enemyParty};
        for (std::size_t i{0}; i < party.size(); ++i)
            if (party.getUnitAt(i) == u)
                return i;
        return 0;
    };

    std::vector<TurnSlot> result{};
    result.reserve(kProjectionSlots);

    while (static_cast<int>(result.size()) < kProjectionSlots)
    {
        // Find unit with minimum AV; tie-break: player before enemy, lower index first.
        float minAV{std::numeric_limits<float>::max()};
        Unit *next{nullptr};
        bool nextIsPlayer{false};
        std::size_t nextIdx{0};

        for (auto &[u, isPlayer] : alive)
        {
            const float av{workAV.at(u)};
            const std::size_t idx{partyIndex(u, isPlayer)};
            bool better{av < minAV};
            if (!better && av == minAV)
            {
                // Tie-breaker 1: player before enemy.
                if (isPlayer && !nextIsPlayer)
                    better = true;
                // Tie-breaker 2: lower formation index.
                else if (isPlayer == nextIsPlayer && idx < nextIdx)
                    better = true;
            }
            if (better)
            {
                minAV = av;
                next = u;
                nextIsPlayer = isPlayer;
                nextIdx = idx;
            }
        }

        if (!next)
            break;

        // Advance all units by minAV (time elapsed until next acts).
        for (auto &[u, av] : workAV)
            av -= minAV;

        result.push_back({next, nextIsPlayer, nextIdx});

        // Reset acting unit's working AV to its base * exposure modifier.
        workAV[next] = computeBaseAV(next) * exposureModifier(next);
    }

    return result;
}

void AVTurnOrderCalculator::applyHasten(Unit *unit, float pct)
{
    auto it{m_currentAV.find(unit)};
    if (it == m_currentAV.end())
        return;
    const float base{m_baseAV.count(unit) ? m_baseAV.at(unit) : computeBaseAV(unit)};
    it->second = std::max(0.0f, it->second - pct * base);
}

void AVTurnOrderCalculator::applySuppress(Unit *unit, float pct)
{
    auto it{m_currentAV.find(unit)};
    if (it == m_currentAV.end())
        return;
    const float base{m_baseAV.count(unit) ? m_baseAV.at(unit) : computeBaseAV(unit)};
    it->second += pct * base;
}

void AVTurnOrderCalculator::onUnitActed(Unit *unit)
{
    auto it{m_currentAV.find(unit)};
    if (it == m_currentAV.end())
        return;

    // Advance all other units by the elapsed time (acting unit's current AV).
    const float elapsed{it->second};
    for (auto &[u, av] : m_currentAV)
        av -= elapsed;

    // Reset acting unit to base AV * exposure modifier.
    const float base{m_baseAV.count(unit) ? m_baseAV.at(unit) : computeBaseAV(unit)};
    it->second = base * exposureModifier(unit);
}