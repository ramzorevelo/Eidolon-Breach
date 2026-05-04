#pragma once
#include "Entities/Enemy.h"

/** Slime: regenerates health every 4th turn. */
class Slime : public Enemy
{
public:
    Slime(std::string name, int maxHp, int maxToughness);

    ActionResult performAttack() override;
    [[nodiscard]] std::string getIntentLabel() const override;

  private:
    int m_turnCount{ 0 };
};