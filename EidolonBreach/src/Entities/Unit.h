#pragma once

/**
 * @file Unit.h
 * @brief Abstract base class for all combatants.
 */

#include "Core/ActionResult.h"
#include "Core/Affinity.h"
#include "Core/Stats.h"
#include "Core/IStatusEffect.h"
#include <string>
#include <memory>
#include <vector>

class Party;   // forward declaration – avoids circular headers

/** Abstract base for all Units (players, enemies, summons). */
class Unit
{
public:
    Unit(std::string id,
         std::string name,
         Stats stats,
         Affinity affinity);

    virtual ~Unit() = default;

    const std::string& getId()                   const;
    const std::string& getName()                 const;
    Affinity           getAffinity()             const;

    /** @return Raw stats with no effect modifiers applied. */
    const Stats &getBaseStats() const;

    /**
     * @brief Compute final stats after applying all active effects (two passes).
     *
     * Pass 1: modifyStatsFlat() on every effect in insertion order.
     * Pass 2: modifyStatsPct() on every effect in insertion order.
     * Flat modifiers always precede percentage multipliers regardless of
     * the order in which effects were applied (see §2.12.3).
     */
    [[nodiscard]] Stats getFinalStats() const;

    int  getHp()    const;
    int  getMaxHp() const;
    bool isAlive()  const;

    void takeDamage(int amount);
    void heal(int amount);

    // Returns ActionResult so Battle can render the outcome.
    virtual ActionResult takeTurn(Party& allies, Party& enemies) = 0;

    // Toughness helpers – default implementations are no-ops so that
    // PlayableCharacter does not need to override them.
    virtual bool isBroken()                const { return false; }
    virtual void applyToughnessHit(int)          {}
    virtual void recoverFromBreak()               {}

protected:
    std::string m_id;
    std::string m_name;
    Stats       m_stats;
    Affinity    m_affinity;
    std::vector<std::unique_ptr<IStatusEffect>> m_effects;
}; 