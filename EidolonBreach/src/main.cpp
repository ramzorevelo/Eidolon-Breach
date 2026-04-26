/**
 * @file main.cpp
 * @brief Entry point. Constructs the player party and runs a dungeon.
 */

#include "Actions/BasicStrikeAction.h"
#include "Actions/SkillAction.h"
#include "Actions/UltimateAction.h"
#include "Summons/Ignis.h"
#include "Summons/SummonRegistry.h"
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

    SummonRegistry summonRegistry{};
    Ignis::registerIgnis(summonRegistry);

    MetaProgress meta{};

    Dungeon dungeon{};
    const std::uint32_t seed{static_cast<std::uint32_t>(std::random_device{}())};
    std::cout << "Run seed: " << seed << '\n'; // Reproduce a specific run
    dungeon.generate(seed, 9, DungeonDifficulty::Normal, &summonRegistry);

    const bool won{dungeon.run(playerParty, meta)};
    std::cout << (won ? "\n=== RUN COMPLETE ===\n" : "\n=== DEFEATED ===\n");

    return 0;
}