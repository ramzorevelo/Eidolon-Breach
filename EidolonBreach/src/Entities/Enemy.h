#pragma once

/**
 * @file Enemy.h
 * @brief Base class for all enemy units.
 */

#include "Entities/Unit.h"
#include "Core/Drop.h"
#include "Entities/IAIStrategy.h"
#include "Core/Drop.h"
#include "Core/Affinity.h"
#include <map>
#include <memory>
#include <optional>

class Enemy : public Unit
{
public:
    Enemy(std::string id,
        std::string name,
        Stats stats,
        Affinity affinity,
        int maxToughness,
        std::unique_ptr<IAIStrategy> aiStrategy,
        std::map<Affinity, float> affinityModifiers = {}
    );

    bool isBroken() const override;
    void applyToughnessHit(int amount) override;
    void recoverFromBreak() override;

    int getToughness() const;
    int getMaxToughness() const;

    float getAffinityModifier(Affinity a) const;


    /**
     * @brief Register a drop entry for this enemy.
     *        Called during enemy construction to build the drop pool.
     */
    void addDrop(Drop drop);

    /**
     * @brief Roll drops and return those that land.
     *        Rolls each Drop::dropChance independently using std::mt19937.
     *        GuaranteedItem drops always land. Called by Battle on enemy defeat.
     * @param seed  RNG seed. Pass a stable per-battle seed for determinism in tests.
     */
    [[nodiscard]] std::vector<Drop> generateDrops(unsigned int seed = 0u) const;


    ActionResult takeTurn(Party &allies, Party &enemies, BattleState &state) override;

protected:
    virtual ActionResult performAttack();

private:
    int m_toughness{};
    int m_maxToughness{};
    bool m_isBroken{ false };
    std::unique_ptr<IAIStrategy> m_aiStrategy;
    std::map<Affinity, float> m_affinityModifiers;
    std::vector<Drop> m_dropPool{};
};