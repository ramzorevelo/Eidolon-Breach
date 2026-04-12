#pragma once
#include "Entities/Enemy.h"

/** Stone Golem: heavy slam every 3rd turn. */
class StoneGolem : public Enemy
{
public:
    StoneGolem(std::string name, int maxHp, int maxToughness);

    ActionResult performAttack() override;

private:
    int m_turnCount{ 0 };
};