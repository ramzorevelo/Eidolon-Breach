#pragma once
#include "Entities/PlayableCharacter.h"
#include "Entities/Enemy.h"
#include "Actions/BasicStrikeAction.h"
#include "Actions/SkillAction.h"
#include "Actions/UltimateAction.h"
#include <memory>

// Creates a minimal hero with all three basic actions.
inline std::unique_ptr<PlayableCharacter> makeHero(const std::string& id = "hero")
{
    auto pc = std::make_unique<PlayableCharacter>(
        id, "Hero",
        Stats{ 120, 120, 15, 0, 10 },
        Affinity::Aether,
        10);
    pc->addAbility(std::make_unique<BasicStrikeAction>());
    pc->addAbility(std::make_unique<SkillAction>());
    pc->addAbility(std::make_unique<UltimateAction>());
    return pc;
}

// Creates a simple enemy with specified HP and toughness.
inline std::unique_ptr<Enemy> makeEnemy(int hp = 100, int toughness = 50)
{
    return std::make_unique<Enemy>(
        "enemy_1", "TestEnemy",
        Stats{ hp, hp, 10, 0, 5 },
        Affinity::Terra,
        toughness,
        std::make_unique<BasicAIStrategy>()
    );
}