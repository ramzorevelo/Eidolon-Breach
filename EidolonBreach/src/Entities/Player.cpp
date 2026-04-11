#include "Entities/Player.h"
#include "Entities/Character.h"
#include "Core/ActionResult.h"
#include <algorithm>

Player::Player(std::string name)
    : Character{ std::move(name), 120 }
{
}
Player::Player(std::string name, int maxHp)
    : Character{ std::move(name), maxHp }
{
}

int  Player::getSp()         const { return m_sp; }
int  Player::getEnergy()     const { return m_energy; }
bool Player::ultimateReady() const { return m_energy >= kMaxEnergy; }

void Player::gainSp(int amount)
{
    m_sp = std::min(kMaxSp, m_sp + amount);
}

void Player::gainEnergy(int amount)
{
    m_energy = std::min(kMaxEnergy, m_energy + amount);
}

ActionResult Player::basicAttack()
{
    gainSp(1);
    gainEnergy(20);
    return ActionResult{ ActionResult::Type::Damage, 15 };
}

ActionResult Player::useSkill()
{
    --m_sp;
    gainEnergy(30);
    return ActionResult{ ActionResult::Type::Damage, 28 };
}

ActionResult Player::useUltimate()
{
    m_energy = 0;
    gainSp(2);
    return ActionResult{ ActionResult::Type::Damage, 60 };
}
