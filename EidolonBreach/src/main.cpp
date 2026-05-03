/**
 * @file main.cpp
 * @brief Entry point. Loads registries from data/ and runs a dungeon.
 */
#include "Dungeon/DungeonTable.h"
#include "Dungeon/DungeonDefinition.h"
#include <limits>
#include "Actions/BasicStrikeAction.h"
#include "Core/RunContext.h"
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
#include "Actions/EmberCallAction.h"
#include "Actions/VexBulwarkAction.h"
#include "Actions/ZaraFrostbindAction.h"
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
    // Lyra's Ember Call 
    reg.registerAbility("lyra_ember_call",
                        []
                        { return std::make_unique<EmberCallAction>(); });
    // Vex's Bulwark
    reg.registerAbility("vex_bulwark",
                        []
                        { return std::make_unique<VexBulwarkAction>(); });
    // Zara's Frostbind
    reg.registerAbility("zara_frostbind",
                        []
                        { return std::make_unique<ZaraFrostbindAction>(); });
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
            const int level{meta.characterLevels.count(id) > 0
                                ? meta.characterLevels.at(id)
                                : 1};
            auto pc{characterRegistry.create(id, level)};
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
        const std::string &id{available[idx - 1]};
        const int level{meta.characterLevels.count(id) > 0
                            ? meta.characterLevels.at(id)
                            : 1};
        auto pc{characterRegistry.create(id, level)};
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

    // --- Mode selection (Classic / Draft) ---
    std::cout << "Mode: [1] Classic  [2] Eidolon Labyrinth\nChoice: ";
    int modeChoice{1};
    std::cin >> modeChoice;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    const RunMode runMode{modeChoice == 2 ? RunMode::EidolonLabyrinth : RunMode::Classic};

    Dungeon dungeon{};
    const DungeonDefinition *selectedDungeon{nullptr};

    if (runMode == RunMode::Classic)
    {
        // --- Classic dungeon selection ---
        const auto &classicDungeons{DungeonTable::getClassicDungeons()};

        std::vector<const DungeonDefinition *> available{};
        bool previousCleared{true};
        for (const auto &def : classicDungeons)
        {
            const bool levelMet{meta.playerLevel >= def.unlockPlayerLevel};
            if (levelMet && previousCleared)
                available.push_back(&def);
            previousCleared = meta.clearedDungeonIds.count(def.id) > 0;
        }
        if (available.empty())
            available.push_back(&classicDungeons.front());

        std::cout << "\n=== DUNGEON SELECT ===\n"
                  << "  Player Level: " << meta.playerLevel << "\n\n";
        for (std::size_t i{0}; i < available.size(); ++i)
        {
            const auto &def{*available[i]};
            const bool cleared{meta.clearedDungeonIds.count(def.id) > 0};
            std::cout << "  [" << (i + 1) << "] " << def.name
                      << "  (Rec. Lv." << def.recommendedPlayerLevel
                      << " | Enemy Lv." << def.enemyLevel
                      << " | " << def.numFloors << " floors"
                      << (cleared ? " | CLEARED" : "") << ")\n"
                      << "      " << def.description << "\n";
        }
        std::cout << "Select dungeon: ";
        int dungeonChoice{1};
        std::cin >> dungeonChoice;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (dungeonChoice < 1 || dungeonChoice > static_cast<int>(available.size()))
            dungeonChoice = 1;
        selectedDungeon = available[static_cast<std::size_t>(dungeonChoice - 1)];
    }
    else
    {
        // Draft mode uses the first dungeon as a default template for now
        selectedDungeon = &DungeonTable::getClassicDungeons().front();
        std::cout << "Eidolon Labyrinth: no XP earned. Attune available at Rest sites.\n";
    }

    dungeon.generate(seed, *selectedDungeon, &summonRegistry, runMode);
    const bool won{dungeon.run(playerParty, meta)};
    std::cout << (won ? "\n=== RUN COMPLETE ===\n" : "\n=== DEFEATED ===\n");

   
    meta.saveToFile("save.json");
    std::cout << "Progress saved. Highest floor reached: "
              << meta.highestFloorReached << '\n';

    return 0;
}