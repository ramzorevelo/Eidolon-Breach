/**
 * @file main.cpp
 * @brief Entry point. Constructs the player party and runs a dungeon.
 */

#include "Actions/BasicStrikeAction.h"
#include "Actions/SkillAction.h"
#include "Actions/UltimateAction.h"
#include "Core/MetaProgress.h"
#include "Dungeon/Dungeon.h"
#include "Entities/PlayableCharacter.h"
#include "Entities/Party.h"
#include <iostream>
#include <memory>

int main()
{
    std::cout << "=== EIDOLON BREACH ===\n"
              << "Striker + Conduit vs the dungeon.\n\n";

    Party playerParty{};
    playerParty.gainSp(50);

    auto striker{std::make_unique<PlayableCharacter>(
        "striker_1", "Striker",
        Stats{80, 80, 22, 5, 14},
        Affinity::Blaze,
        10)};
    striker->addAbility(std::make_unique<BasicStrikeAction>());
    striker->addAbility(std::make_unique<SkillAction>(2.0f));
    striker->addAbility(std::make_unique<UltimateAction>());
    playerParty.addUnit(std::move(striker));

    auto conduit{std::make_unique<PlayableCharacter>(
        "conduit_1", "Conduit",
        Stats{110, 110, 10, 15, 9},
        Affinity::Terra,
        8)};
    conduit->addAbility(std::make_unique<BasicStrikeAction>());
    conduit->addAbility(std::make_unique<SkillAction>(2.0f));
    conduit->addAbility(std::make_unique<UltimateAction>());
    playerParty.addUnit(std::move(conduit));

    MetaProgress meta{};

    Dungeon dungeon{};
    dungeon.generate(12345u, 9, DungeonDifficulty::Normal);

    const bool won{dungeon.run(playerParty, meta)};
    std::cout << (won ? "\n=== RUN COMPLETE ===\n" : "\n=== DEFEATED ===\n");

    return 0;
}