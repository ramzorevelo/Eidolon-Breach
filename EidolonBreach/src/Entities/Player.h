#pragma once

#include "Entities/Character.h"
#include "Core/ActionResult.h"

// Represents the player. Inherits HP management from Character and
// adds the SP / energy resource system with three distinct action tiers.
class Player : public Character
{
public:
    static constexpr int kMaxSp{ 5 };
    static constexpr int kMaxEnergy{ 100 };

    explicit Player(std::string name);

    int  getSp()         const;
    int  getEnergy()     const;
    bool ultimateReady() const;

    // Each action returns the HP damage to deal to the enemy.
    // Toughness (break gauge) damage is applied by Battle.
    ActionResult basicAttack();  // always available: +1 SP, +20 energy
    virtual ActionResult useSkill();     // costs 1 SP:       -1 SP, +30 energy
    ActionResult useUltimate(); // requires full energy: energy -> 0, +2 SP

protected:
    Player(std::string name, int maxHp);

private:
    int m_sp{ 3 };
    int m_energy{ 0 };

    void gainSp(int amount);
    void gainEnergy(int amount);

};


