#pragma once
#include "Entities/Enemy.h"

class Slime : public Enemy
{
public:
    Slime(std::string name, int maxHp, int maxToughness);

    ActionResult performAttack() override;

private:
    int m_turnCount{ 0 };
};