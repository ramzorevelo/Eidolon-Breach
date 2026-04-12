#pragma once
#include "Entities/Enemy.h"

/** Vampire Bat: lifedrain attack every 3rd turn. */
class VampireBat : public Enemy
{
public:
    VampireBat(std::string name, int maxHp, int maxToughness);

    ActionResult performAttack() override;

private:
    int m_turnCount{ 0 };
};