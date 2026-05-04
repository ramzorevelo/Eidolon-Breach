#pragma once

/**
 * @file Enemy.h
 * @brief Base class for all enemy units.
 */

#include "Core/Affinity.h"
#include "Core/CombatConstants.h"
#include "Core/Drop.h"
#include "Entities/IAIStrategy.h"
#include "Entities/Unit.h"
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

struct BattleState;
class Enemy;

/**
 * @brief Fired once when an enemy's Toughness gauge first hits zero.
 *        Battle calls onBreak immediately after detecting isBroken().
 *        state.playerParty / state.enemyParty are nullptr until Phase 6;
 *        all callbacks must null-check before dereferencing them.
 */
struct BreakEffect
{
    std::string id{};
    std::string displayName{};
    std::function<void(Enemy &, BattleState &)> onBreak{};

    BreakEffect() = default;
    BreakEffect(std::string id_,
                std::string displayName_,
                std::function<void(Enemy &, BattleState &)> callback)
        : id{std::move(id_)}, displayName{std::move(displayName_)}, onBreak{std::move(callback)}
    {
    }
};

class Enemy : public Unit
{
  public:
    Enemy(std::string id,
          std::string name,
          Stats stats,
          Affinity affinity,
          int maxToughness,
          std::unique_ptr<IAIStrategy> aiStrategy,
          std::map<Affinity, float> affinityModifiers = {});

    // Toughness
    bool isBroken() const override;
    /**
     * @brief Reduce toughness by amount × getAffinityModifier(sourceAffinity).
     *        Sets isBroken() and resets toughness to max when gauge hits 0.
     *        The BreakEffect callback is fired by Battle, not here.
     */
    void applyToughnessHit(int amount, Affinity sourceAffinity = Affinity::Aether) override;
    void recoverFromBreak() override;
    [[nodiscard]] float getToughnessAffinityModifier(Affinity affinity) const override;

    [[nodiscard]] int getToughness() const;
    [[nodiscard]] int getMaxToughness() const;
    /**
     * @brief Scale maxToughness (and current toughness) by the given factor.
     *        Used by BattleNode to apply floor-affinity toughness modifiers at spawn.
     * @param factor Multiplier (e.g. 1.10f for +10%, 0.90f for -10%).
     */
    void scaleMaxToughness(float factor);
    [[nodiscard]] float getAffinityModifier(Affinity a) const;

    // Break effect
    void setBreakEffect(BreakEffect effect);
    [[nodiscard]] const BreakEffect &getBreakEffect() const;
    [[nodiscard]] float getBrokenDamageBonus() const;

    // Drops
    void addDrop(Drop drop);
    [[nodiscard]] std::vector<Drop> generateDrops(unsigned int seed = 0u) const;

    ActionResult takeTurn(Party &allies, Party &enemies, BattleState &state) override;

    /**
     * @brief Overrides Unit::takeDamage to apply the broken damage bonus
     *        (kBrokenDamageBonus × amount) while isBroken() is true.
     */
    void takeDamage(int amount) override;

    /**
     * @brief Overrides Unit::takeTrueDamage with the same broken bonus logic.
     *        DoT that fires during the break window still benefits the party.
     */
    void takeTrueDamage(int amount) override;

    /**
     * @brief Called by the enemy's own BreakEffect callback for per-subclass
     *        post-break behaviour (e.g. VampireBat activates bloodless).
     *        Default: no-op.
     */
    virtual void onBreakCallback() {}

    [[nodiscard]] std::string getIntentLabel() const override;

  protected:
    virtual ActionResult performAttack();

  private:
    int m_toughness{};
    int m_maxToughness{};
    bool m_isBroken{false};
    int m_brokenTurnsRemaining{0};
    float m_brokenDamageBonus{CombatConstants::kBrokenDamageBonus};
    BreakEffect m_breakEffect{};
    std::unique_ptr<IAIStrategy> m_aiStrategy;
    std::map<Affinity, float> m_affinityModifiers;
    std::vector<Drop> m_dropPool{};
};