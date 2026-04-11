#include "Character.h"
#include <algorithm>
#include <utility>

Character::Character(std::string name, int maxHp)
    : m_name{ std::move(name) }
    , m_hp{ maxHp }
    , m_maxHp{ maxHp }
{
}

const std::string& Character::getName()  const { return m_name; }
int  Character::getHp()    const { return m_hp; }
int  Character::getMaxHp() const { return m_maxHp; }
bool Character::isAlive()  const { return m_hp > 0; }

void Character::takeDamage(int amount)
{
    m_hp = std::max(0, m_hp - amount);
}

void Character::heal(int amount)
{
    m_hp = std::min(m_maxHp, m_hp + amount);
}
