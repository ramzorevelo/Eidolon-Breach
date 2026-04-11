#pragma once

#include "Entities/Enemy.h"

// A specific enemy type that derives from Enemy.
// Every fourth turn it regenerates 15 HP instead of attacking.
// All other turns it deals 12 damage.

class Slime : public Enemy
{
public:
    Slime(std::string name, int maxHp, int maxToughness);

    // Every fourth call heals the Slime for 15 HP and returns 0 damage.
    // All other calls deal 12 damage.
    // override tells the compiler to verify this matches a virtual in Enemy.
    ActionResult performAttack() override;

private:
    int m_turnCount{ 0 };
};