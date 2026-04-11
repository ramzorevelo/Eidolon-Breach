#pragma once
#include <string>

// Base class shared by the Player and Enemy.
// Owns the character's name, HP, and the logic that changes HP.
class Character
{
public:
    Character(std::string name, int maxHp);
    virtual ~Character() = default;

    const std::string& getName()  const;
    int  getHp()    const;
    int  getMaxHp() const;
    bool isAlive()  const;

    void takeDamage(int amount);
    void heal(int amount);

protected:
    std::string m_name{};
    int m_hp{};
    int m_maxHp{};
};
