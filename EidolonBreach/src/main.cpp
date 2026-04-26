/**
 * @file main.cpp
 * @brief Entry point. Loads registries from data/ and runs a dungeon.
 */

#include "Actions/BasicStrikeAction.h"
#include "Actions/SkillAction.h"
#include "Actions/UltimateAction.h"
#include "Characters/AbilityRegistry.h"
#include "Characters/CharacterRegistry.h"
#include "Core/MetaProgress.h"
#include "Dungeon/Dungeon.h"
#include "Entities/PlayableCharacter.h"
#include "Entities/Enemy.h"
#include "Entities/Party.h"
#include "Summons/Ignis.h"
#include "Summons/SummonRegistry.h"
#include <iostream>
#include <memory>

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
    // Add new abilities here as new characters are defined.
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
    MetaProgress meta{};
    Dungeon dungeon{};
    dungeon.generate(12345u, 9, DungeonDifficulty::Normal, &summonRegistry);

    const bool won{dungeon.run(playerParty, meta)};
    std::cout << (won ? "\n=== RUN COMPLETE ===\n" : "\n=== DEFEATED ===\n");

    return 0;
}