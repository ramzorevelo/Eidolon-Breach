#pragma once
#include "Core/ActionResult.h"
#include "Core/Affinity.h"
#include "Core/Stats.h"
#include <string>

class Party;   // forward declaration – avoids circular headers

class Unit
{
public:
    Unit(std::string id,
         std::string name,
         Stats       stats,
         Affinity    affinity,
         int         resonanceContribution,
         std::string passiveTrait = "");

    virtual ~Unit() = default;

    // ── Identity ──────────────────────────────────────────────────────
    const std::string& getId()                   const;
    const std::string& getName()                 const;
    Affinity           getAffinity()             const;
    int                getResonanceContribution() const;
    const std::string& getPassiveTrait()          const;

    // ── Stats ─────────────────────────────────────────────────────────
    const Stats& getStats() const;
    int  getHp()    const;
    int  getMaxHp() const;
    bool isAlive()  const;

    void takeDamage(int amount);
    void heal(int amount);

    // ── Combat virtuals ───────────────────────────────────────────────
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
    int         m_resonanceContribution;
    std::string m_passiveTrait;
};