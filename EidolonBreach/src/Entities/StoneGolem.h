#pragma once

#include "Entities/Enemy.h"

// A specific enemy type that derives from Enemy.
// Overrides performAttack() with a heavy-strike pattern every third turn.
class StoneGolem : public Enemy
{
public:
    StoneGolem(std::string name, int maxHp, int maxToughness);

    // Every third call deals 35 damage (heavy ground slam).
    // All other calls deal 20 damage.
    // override tells the compiler to verify this matches a virtual in Enemy.
    ActionResult performAttack() override;

private:
    int m_turnCount{ 0 };
};
