#pragma once

/**
 * @file Unit.h
 * @brief Abstract base class for all combatants.
 */

#include "Core/ActionResult.h"
#include "Core/Affinity.h"
#include "Core/Drop.h"
#include "Core/Stats.h"
#include "Core/IStatusEffect.h"
#include <string>
#include <memory>
#include <vector>

class Party;   
struct BattleState;
class PlayableCharacter;
/** Abstract base for all Units (players, enemies, summons). */
class Unit
{
public:
    Unit(std::string id,
         std::string name,
         Stats stats,
         Affinity affinity);

    virtual ~Unit() = default;

    [[nodiscard]] const std::string &getId() const;
    [[nodiscard]] const std::string &getName() const;
    [[nodiscard]] Affinity getAffinity() const;
    [[nodiscard]] const Stats &getBaseStats() const;


    /**
     * @brief Compute final stats after applying all active effects (two passes).
     *
     * Pass 1: modifyStatsFlat() on every effect in insertion order.
     * Pass 2: modifyStatsPct() on every effect in insertion order.
     * Flat modifiers always precede percentage multipliers regardless of
     * the order in which effects were applied.
     */
    [[nodiscard]] virtual Stats getFinalStats() const;

    [[nodiscard]] int getHp() const;
    [[nodiscard]] int getMaxHp() const;
    [[nodiscard]] bool isAlive() const;

    /**
     * @brief Reduce HP by amount, first running the shield absorption pass.
     *
     * Shield effects absorb damage before it reaches HP. Exhausted shields
     * remain in m_effects until the next tickEffects() cleans them up.
     * For DoT that should bypass shields, call takeTrueDamage() instead.
     */
    virtual void takeDamage(int amount);
    void heal(int amount);

    /**
     * @brief Scale maxHp and atk by the given multipliers.
     *        hp is reset to the new maxHp after scaling.
     *        Called by BattleNode when applying difficulty modifiers at spawn.
     */
    void scaleStats(float hpMultiplier, float atkMultiplier);

    /**
     * @brief Reduces HP without running the shield absorption pass.
     * Use for DoT ticks (BurnEffect::onTick). Direct attacks call takeDamage().
     */
    virtual void takeTrueDamage(int amount);

    //  Effect Management 
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
    [[nodiscard]] bool hasEffect(std::string_view id) const;

    /** @return true if any active effect carries the given tag. */
    [[nodiscard]] bool hasEffectWithTag(std::string_view tag) const;

    /** @return Total remaining shield absorb amount from all active shield effects. */
    [[nodiscard]] int getTotalShieldAmount() const;

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
     * @return true when this unit is a player-side Manifestation (Summon).
     *         Used by Battle to count active summons and enforce the party cap.
     */
    [[nodiscard]] virtual bool isSummon() const
    {
        return false;
    }

    /**
     * @return A short label describing what this unit intends to do on its
     *         next turn (e.g. "Attacks", "Regenerates"). Empty string = no intent shown.
     */
    [[nodiscard]] virtual std::string getIntentLabel() const
    {
        return {};
    }
    /**
     * @brief Called by Battle at the end of combat for cleanup.
     *        PlayableCharacter overrides to reset consumable state and cooldowns.
     *        Default: no-op.
     */
    virtual void onBattleReset() {}

    /**
     * @brief Tick summon-specific lifecycle (duration countdown).
     *        Summon overrides; returns true if the summon has expired.
     *        Default: returns false.
     */
    [[nodiscard]] virtual bool tickSummonLifecycle()
    {
        return false;
    }

    /**
     * @brief Generate drops for defeated units.
     *        Enemy overrides to produce its drop table.
     *        Default: returns empty vector.
     * @param seed RNG seed for drop rolls.
     */
    [[nodiscard]] virtual std::vector<Drop> generateDropsForBattle(unsigned int seed) const
    {
        (void)seed;
        return {};
    }

    /**
     * @brief Fire this unit's break callback if one is set.
     *        Enemy overrides to invoke its BreakEffect::onBreak with itself.
     *        Default: no-op. Keeps Battle free of Enemy* casts.
     */
    virtual void triggerBreakEffect(BattleState & /*state*/) {}

    /**
     * @brief Gain energy if applicable. Only meaningful for PlayableCharacter.
     *        Default: no-op.
     */
    virtual void gainEnergyIfApplicable(int /*amount*/) {}

    /**
     * @brief Scale max toughness by a factor. Only meaningful for Enemy.
     *        Default: no-op.
     */
    virtual void scaleMaxToughness(float /*factor*/) {}

    /**
     * @brief Polymorphic accessor. Returns this as PlayableCharacter if applicable.
     *        Default: returns nullptr.
     */
    [[nodiscard]] virtual PlayableCharacter *asPlayableCharacter()
    {
        return nullptr;
    }

    /**
     * @brief Clears the broken flag if set. Called by Battle at the start of
     *        an enemy's turn slot. Returns true if broken state was cleared.
     *        Default: no-op (PCs and Summons are never broken).
     */
    virtual bool checkAndClearBroken()
    {
        return false;
    }

    /**
     * @return The unit's Resonance Field contribution value.
     *         Base implementation returns 0 (enemies never contribute).
     *         Overridden by PlayableCharacter and Summon.
     */
    [[nodiscard]] virtual int getResonanceContribution() const
    {
        return 0;
    }

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
    uint16_t m_activeTags{0};
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
    void rebuildTagCache();
}; 