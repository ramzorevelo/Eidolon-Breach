#pragma once
#include "Entities/Character.h"
#include "Core/ActionResult.h"
#include <optional>
#include "Core/Drop.h"

// Represents a single enemy unit.
// Inherits HP from Character and adds a Toughness break gauge.
class Enemy : public Character
{
public:
    Enemy(std::string name, int maxHp, int maxToughness, std::optional<Drop> drop = std::nullopt);
    int  getToughness()    const;
    int  getMaxToughness() const;
    bool isBroken()        const;
    bool hasDrop() const;
    const std::optional<Drop>& getDrop() const;
    std::optional<Drop> dropLoot();

    // Reduces the break gauge by amount.
    // Sets isBroken() and resets the gauge when it reaches zero.
    void reduceToughness(int amount);

    // Called at the end of the enemy's stunned turn to clear the broken flag.
    void recoverFromBreak();

    // Returns the HP damage dealt to the player this turn.     
    // // Derived classes override this to implement specific attack patterns.
    virtual ActionResult performAttack();




private:
    int  m_toughness{};
    int  m_maxToughness{};
    bool m_isBroken{ false };
    std::optional<Drop> m_drop{};
};

