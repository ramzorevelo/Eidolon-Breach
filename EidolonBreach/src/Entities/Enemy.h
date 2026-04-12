#pragma once
#include "Entities/Unit.h"
#include "Entities/IAIStrategy.h"
#include "Core/Drop.h"
#include "Core/Affinity.h"
#include <map>
#include <memory>
#include <optional>

/**
 * @file Enemy.h
 * @brief Base class for all enemy units.
 */
class Enemy : public Unit
{
public:
    Enemy(std::string id,
        std::string name,
        Stats stats,
        Affinity affinity,
        int maxToughness,
        std::unique_ptr<IAIStrategy> aiStrategy,
        std::map<Affinity, float> affinityModifiers = {},
        std::optional<Drop> drop = std::nullopt);

    bool isBroken() const override;
    void applyToughnessHit(int amount) override;
    void recoverFromBreak() override;

    int getToughness() const;
    int getMaxToughness() const;

    float getAffinityModifier(Affinity a) const;

    bool hasDrop() const;
    const std::optional<Drop>& getDrop() const;
    std::optional<Drop> dropLoot();

    ActionResult takeTurn(Party& allies, Party& enemies) override;

protected:
    virtual ActionResult performAttack();

private:
    int m_toughness{};
    int m_maxToughness{};
    bool m_isBroken{ false };
    std::unique_ptr<IAIStrategy> m_aiStrategy;
    std::map<Affinity, float> m_affinityModifiers;
    std::optional<Drop> m_drop{};
};