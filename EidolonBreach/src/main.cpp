/**
 * @file main.cpp
 * @brief Entry point. Loads registries from data/ and runs a dungeon.
 */

#include "Actions/BasicStrikeAction.h"
#include "Actions/SkillAction.h"
#include "Actions/UltimateAction.h"
#include "Characters/AbilityRegistry.h"
#include "Characters/CharacterRegistry.h"
#include "Core/CombatConstants.h"
#include "Core/MetaProgress.h"
#include "Dungeon/Dungeon.h"
#include "Entities/PlayableCharacter.h"
#include "Entities/Enemy.h"
#include "Entities/Party.h"
#include "Summons/Ignis.h"
#include "Summons/SummonRegistry.h"
#include <algorithm>
#include <iostream>
#include <memory>
#include <random>

/** Register all playable abilities before the CharacterRegistry loads characters. */
static AbilityRegistry buildAbilityRegistry()
{
    AbilityRegistry reg{};
    reg.registerAbility("basic_strike",
                        []
                        { return std::make_unique<BasicStrikeAction>(); });
    reg.registerAbility("arch_skill_default",
                        []
                        { return std::make_unique<SkillAction>(2.0f); });
    reg.registerAbility("ultimate_default",
                        []
                        { return std::make_unique<UltimateAction>(); });
    return reg;
}

int main()
{
    std::cout << "=== EIDOLON BREACH ===\n\n";

    // --- Registries ---
    AbilityRegistry abilityRegistry{buildAbilityRegistry()};

    CharacterRegistry characterRegistry{};
    characterRegistry.loadFromJson("data/characters.json", abilityRegistry);

    SummonRegistry summonRegistry{};
    Ignis::registerIgnis(summonRegistry);

    // --- MetaProgress ---
    MetaProgress meta{MetaProgress::loadFromFile("save.json")};

    // Unlock starting characters if this is a fresh save.
    for (const std::string &id : characterRegistry.getIds())
        meta.unlockCharacter(id);

    // --- Party ---
    Party playerParty{};
    playerParty.gainSp(50);

    for (const std::string &id : characterRegistry.getIds())
    {
        auto pc{characterRegistry.create(id)};
        if (pc)
            playerParty.addUnit(std::move(pc));
    }

    // --- Run ---
    const std::uint32_t seed{static_cast<std::uint32_t>(std::random_device{}())};
    std::cout << "Run seed: " << seed << '\n';

    Dungeon dungeon{};
    dungeon.generate(seed, 9, DungeonDifficulty::Normal, &summonRegistry);

    const bool won{dungeon.run(playerParty, meta)};
    std::cout << (won ? "\n=== RUN COMPLETE ===\n" : "\n=== DEFEATED ===\n");

    meta.highestFloorReached = std::max(meta.highestFloorReached,
                                        static_cast<int>(won ? 9 : 0));
    meta.saveToFile("save.json");
    std::cout << "Progress saved. Highest floor reached: "
              << meta.highestFloorReached << '\n';

    return 0;
}