#pragma once
#include "Entities/Enemy.h"

class VampireBat : public Enemy
{
public:
    VampireBat(std::string name, int maxHp, int maxToughness);
    ActionResult performAttack() override;

private:
    int m_turnCount{ 0 };
};