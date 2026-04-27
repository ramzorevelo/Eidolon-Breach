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
#include "Entities/Party.h"
#include "Entities/PlayableCharacter.h"
#include "Summons/Ignis.h"
#include "Summons/SummonRegistry.h"
#include <algorithm>
#include <iostream>
#include <memory>
#include <random>
#include <sstream>

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

/**
 * @brief Present a character selection menu and populate the player party.
 *        Auto-selects when available characters <= kMaxPlayerCharacters.
 */
static void selectParty(Party &party,
                        const CharacterRegistry &characterRegistry,
                        const MetaProgress &meta)
{
    std::vector<std::string> available{};
    for (const std::string &id : characterRegistry.getIds())
        if (meta.unlockedCharacterIds.count(id) > 0)
            available.push_back(id);

    if (available.empty())
    {
        std::cout << "No characters unlocked. Cannot start run.\n";
        return;
    }

    // Auto-select when there is no real choice to make.
    if (available.size() <= static_cast<std::size_t>(CombatConstants::kMaxPlayerCharacters))
    {
        for (const std::string &id : available)
        {
            auto pc{characterRegistry.create(id)};
            if (pc)
                party.addUnit(std::move(pc));
        }
        return;
    }

    // Display selection menu.
    std::cout << "\n=== CHARACTER SELECT ===\n";
    std::cout << "Choose up to " << CombatConstants::kMaxPlayerCharacters
              << " characters (enter numbers separated by spaces, e.g. 1 2):\n\n";

    for (std::size_t i{0}; i < available.size(); ++i)
    {
        const std::string &id{available[i]};
        auto pc{characterRegistry.create(id)};
        if (!pc)
            continue;

        const int level{meta.characterLevels.count(id) > 0
                            ? meta.characterLevels.at(id)
                            : 1};
        const Stats &s{pc->getBaseStats()};

        std::cout << "  [" << (i + 1) << "] "
                  << pc->getName()
                  << " [" << characterRegistry.getArchetype(available[i]) << "]"
                  << "  Lv." << level
                  << "  HP:" << s.maxHp
                  << " ATK:" << s.atk
                  << " DEF:" << s.def
                  << " SPD:" << s.spd
                  << '\n';
    }

    std::cout << "\nSelection: ";

    std::vector<std::size_t> chosen{};
    std::string line{};
    std::getline(std::cin, line);
    std::istringstream iss{line};
    std::size_t input{};
    while (iss >> input)
    {
        if (input >= 1 && input <= available.size())
            if (std::find(chosen.begin(), chosen.end(), input) == chosen.end())
                chosen.push_back(input);
        if (chosen.size() >= static_cast<std::size_t>(CombatConstants::kMaxPlayerCharacters))
            break;
    }

    // Fallback: auto-pick first character if nothing valid entered.
    if (chosen.empty())
        chosen.push_back(1);

    for (std::size_t idx : chosen)
    {
        auto pc{characterRegistry.create(available[idx - 1])};
        if (pc)
            party.addUnit(std::move(pc));
    }
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

    selectParty(playerParty, characterRegistry, meta);

    if (playerParty.size() == 0)
    {
        std::cout << "No characters selected. Exiting.\n";
        return 0;
    }

    std::cout << "\nStarting run with " << playerParty.size() << " character(s).\n";

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