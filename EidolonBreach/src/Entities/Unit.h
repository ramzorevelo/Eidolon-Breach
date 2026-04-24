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

class Party;   
struct BattleState;
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
    [[nodiscard]] virtual Stats getFinalStats() const;

    int  getHp()    const;
    int  getMaxHp() const;
    bool isAlive()  const;

    /**
     * @brief Reduce HP by amount, first running the shield absorption pass.
     *
     * Shield effects absorb damage before it reaches HP. Exhausted shields
     * remain in m_effects until the next tickEffects() cleans them up.
     * For DoT that should bypass shields, call takeTrueDamage() instead.
     */
    virtual void takeDamage(int amount);
    void heal(int amount);

        // --- Direct damage (bypasses shield absorption — for DoT effects) ---
    /**
     * @brief Reduces HP without running the shield absorption pass.
     * Use for DoT ticks (BurnEffect::onTick). Direct attacks call takeDamage().
     */
    virtual void takeTrueDamage(int amount);

    // --- Effect Management ---
    /**
     * @brief Apply a status effect to this unit.
     *
     * If an effect with the same ID already exists, it is replaced (refresh
     * semantics — prevents infinite stacking of the same effect).
     * onApply() is called on the incoming effect after placement.
     */
    void applyEffect(std::unique_ptr<IStatusEffect> effect);

    /** @brief Remove the effect with the given ID, calling onRemove(). No-op if absent. */
    void removeEffect(std::string_view id);

    /** @brief Remove all effects that carry the given tag, calling onRemove() on each. */
    void removeEffectsByTag(std::string_view tag);

    /** @brief Extend duration of all effects with the given tag by the specified turns. */
    void extendEffectsByTag(std::string_view tag, int turns);

    /** @return true if an effect with the given ID is currently active. */
    bool hasEffect(std::string_view id) const;

    /** @return true if any active effect carries the given tag. */
    bool hasEffectWithTag(std::string_view tag) const;

    /**
     * @brief Tick all active effects, decrement durations, remove expired ones.
     *
     * Called by Battle::run() at the start of each unit's turn.
     * Snapshots raw pointers before iterating — onTick() may exhaust shields
     * without invalidating the loop.
     *
     * @return Human-readable messages from effects that produced output (e.g. DoT damage).
     */
    [[nodiscard]] std::vector<std::string> tickEffects();

    /** @return Read-only view of active effects (for rendering). */
    const std::vector<std::unique_ptr<IStatusEffect>> &getEffects() const;

    // Forward declaration so the signature compiles without including BattleState.h.
    virtual ActionResult takeTurn(Party &allies, Party &enemies, BattleState &state) = 0;

    // Toughness helpers – default implementations are no-ops so that
    // PlayableCharacter does not need to override them.
    virtual bool isBroken()                const { return false; }
    virtual void applyToughnessHit(int, Affinity = Affinity::Aether) {}
    virtual void recoverFromBreak()               {}

    /**
     * @brief Returns the toughness affinity modifier for the given affinity.
     *        Base implementation returns 1.0f (neutral — no weakness or resistance).
     *        Enemy overrides this to expose its affinity modifier map without
     *        requiring a downcast at call sites in Vestiges/.
     */
    [[nodiscard]] virtual float getToughnessAffinityModifier(Affinity /*affinity*/) const
    {
        return 1.0f;
    }

protected:
    std::string m_id;
    std::string m_name;
    Stats       m_stats;
    Affinity    m_affinity;
    std::vector<std::unique_ptr<IStatusEffect>> m_effects;
    /**
     * @brief Returns the flat affinity resistance for the given affinity.
     *        Base implementation returns 0. PlayableCharacter overrides to
     *        check equipment AffinityResistance effects.
     * @param affinity The affinity of the incoming attack.
     */
    [[nodiscard]] virtual int getAffinityResistance(Affinity affinity) const
    {
        (void)affinity;
        return 0;
    }
}; 