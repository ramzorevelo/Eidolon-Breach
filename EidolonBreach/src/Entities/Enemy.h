#pragma once
#include "Entities/Unit.h"
#include "Entities/IAIStrategy.h"
#include "Core/Drop.h"
#include "Core/Affinity.h"
#include <map>
#include <memory>
#include <optional>

class Enemy : public Unit
{
public:
    Enemy(std::string                      id,
        std::string                      name,
        Stats                            stats,
        Affinity                         affinity,
        int                              maxToughness,
        std::map<Affinity, float>        affinityModifiers = {},
        std::optional<Drop>              drop = std::nullopt);

    // ── Toughness (overrides from Unit) ───────────────────────────────
    bool isBroken()              const override;
    void applyToughnessHit(int amount) override;   // applies modifier; breaks if gauge hits 0
    void recoverFromBreak()            override;

    int  getToughness()    const;
    int  getMaxToughness() const;

    // ── Affinity modifiers (wired in Phase 4) ────────────────────────
    float getAffinityModifier(Affinity a) const;

    // ── Drops ─────────────────────────────────────────────────────────
    bool                       hasDrop()  const;
    const std::optional<Drop>& getDrop()  const;
    std::optional<Drop>        dropLoot();          // clears m_drop after use

    // ── Turn ─────────────────────────────────────────────────────────
    // Uses BasicAIStrategy: attacks first alive player unit.
    ActionResult takeTurn(Party& allies, Party& enemies) override;

protected:
    // Subclasses override this to define attack patterns.
    // Returns an ActionResult describing the attack.
    virtual ActionResult performAttack();

private:
    int                       m_toughness{};
    int                       m_maxToughness{};
    bool                      m_isBroken{ false };
    std::map<Affinity, float> m_affinityModifiers;
    std::optional<Drop>       m_drop{};
};