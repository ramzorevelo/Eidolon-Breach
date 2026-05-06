/**
 * @file main.cpp
 * @brief Entry point. Loads registries from data/ and runs a dungeon.
 */
#include "Actions/BasicStrikeAction.h"
#include "Actions/SkillAction.h"
#include "Actions/UltimateAction.h"
#include "Characters/AbilityRegistry.h"
#include "Characters/CharacterRegistry.h"
#include "Characters/Lyra/EmberCallAction.h"
#include "Characters/Lyra/Ignis.h"
#include "Characters/Lyra/LyraUltimateAction.h"
#include "Characters/Vex/VexBulwarkAction.h"
#include "Characters/Vex/VexUltimateAction.h"
#include "Characters/Zara/ZaraFrostbindAction.h"
#include "Characters/Zara/ZaraUltimateAction.h"
#include "Core/CombatConstants.h"
#include "Core/MetaProgress.h"
#include "Core/RunContext.h"
#include "Dungeon/Dungeon.h"
#include "Dungeon/DungeonDefinition.h"
#include "Dungeon/DungeonTable.h"
#include "Entities/Party.h"
#include "Entities/PlayableCharacter.h"
#include "UI/IInputHandler.h"
#include "Summons/SummonRegistry.h"
#include "UI/SDL3InputHandler.h"
#include "UI/SDL3Renderer.h"
#include <algorithm>
#include <memory>
#include <random>

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
    reg.registerAbility("lyra_ultimate",
                        []
                        { return std::make_unique<LyraUltimateAction>(); });
    reg.registerAbility("vex_ultimate",
                        []
                        { return std::make_unique<VexUltimateAction>(); });
    reg.registerAbility("zara_ultimate",
                        []
                        { return std::make_unique<ZaraUltimateAction>(); });
    reg.registerAbility("lyra_ember_call",
                        []
                        { return std::make_unique<EmberCallAction>(); });
    reg.registerAbility("vex_bulwark",
                        []
                        { return std::make_unique<VexBulwarkAction>(); });
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
                        const MetaProgress &meta,
                        IRenderer &renderer,
                        IInputHandler &input)
{
    std::vector<std::string> available{};
    for (const std::string &id : characterRegistry.getIds())
        if (meta.unlockedCharacterIds.count(id) > 0)
            available.push_back(id);

    if (available.empty())
    {
        renderer.renderMessage("No characters unlocked. Cannot start run.");
        renderer.presentPause(1000);
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

    // Build options list for the menu.
    std::vector<std::string> options{};
    for (std::size_t i = 0; i < available.size(); ++i)
    {
        const auto pc = characterRegistry.create(available[i]);
        if (!pc)
            continue;
        const int level = meta.characterLevels.count(available[i]) > 0
                              ? meta.characterLevels.at(available[i])
                              : 1;
        const Stats &s = pc->getBaseStats();
        options.push_back(pc->getName() + " [" + characterRegistry.getArchetype(available[i]) + "]" + "  Lv." + std::to_string(level) + "  HP:" + std::to_string(s.maxHp) + " ATK:" + std::to_string(s.atk) + " SPD:" + std::to_string(s.spd));
    }

    input.setMenuContext("CHARACTER SELECT", options);
    renderer.renderSelectionMenu("CHARACTER SELECT", options);
    const std::size_t pick = input.getMenuChoice(options.size());

    const std::string &chosenId = available[pick];
    const int level = meta.characterLevels.count(chosenId) > 0
                          ? meta.characterLevels.at(chosenId)
                          : 1;
    auto pc = characterRegistry.create(chosenId, level);
    if (pc)
        party.addUnit(std::move(pc));
}

int main()
{
    // Registries
    AbilityRegistry abilityRegistry{buildAbilityRegistry()};

    CharacterRegistry characterRegistry{};
    characterRegistry.loadFromJson("data/characters.json", abilityRegistry);

    SummonRegistry summonRegistry{};
    Ignis::registerIgnis(summonRegistry);

    SDL3Renderer renderer{"Eidolon Breach", 1280, 720};
    SDL3InputHandler input{renderer};

    // MetaProgress
    MetaProgress meta{MetaProgress::loadFromFile("save.json")};

    // Unlock starting characters if this is a fresh save.
    for (const std::string &id : characterRegistry.getIds())
        std::ignore = meta.unlockCharacter(id);

    // Party
    Party playerParty{};
    playerParty.gainSp(50);

    try
    {
    selectParty(playerParty, characterRegistry, meta, renderer, input);

    if (playerParty.size() == 0)
    {
        renderer.renderMessage("No characters selected. Exiting.");
        renderer.presentPause(1000);
        return 0;
    }

    renderer.renderMessage("Starting run with " + std::to_string(playerParty.size()) + " character(s).");

    // Run seed
    const std::uint32_t seed{static_cast<std::uint32_t>(std::random_device{}())};
    renderer.renderMessage("Run seed: " + std::to_string(seed));
    renderer.presentPause(600);

    // Mode selection
    const std::vector<std::string> modeOptions{
        "Classic",
        "Eidolon Labyrinth"};
    input.setMenuContext("SELECT MODE", modeOptions);
    renderer.renderSelectionMenu("SELECT MODE", modeOptions);
    const std::size_t modeChoice = input.getMenuChoice(modeOptions.size());
    const RunMode runMode{modeChoice == 1
                              ? RunMode::EidolonLabyrinth
                              : RunMode::Classic};

    Dungeon dungeon{};
    const DungeonDefinition *selectedDungeon{nullptr};

    if (runMode == RunMode::Classic)
    {
        // Build list of available dungeons.
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

        // Build options for dungeon select menu.
        std::vector<std::string> dungeonOptions{};
        for (const auto *def : available)
        {
            const bool cleared{meta.clearedDungeonIds.count(def->id) > 0};
            const int displayFloors{def->fixedLayout.empty()
                                        ? def->numFloors
                                        : static_cast<int>(def->fixedLayout.size())};
            dungeonOptions.push_back(
                def->name + "  Rec.Lv." + std::to_string(def->recommendedPlayerLevel) + " | EnemyLv." + std::to_string(def->enemyLevel) + " | " + std::to_string(displayFloors) + " floors" + (cleared ? " [CLEARED]" : ""));
        }

        const std::string dungeonTitle = "DUNGEON SELECT  Player Lv." + std::to_string(meta.playerLevel);
        input.setMenuContext(dungeonTitle, dungeonOptions);
        renderer.renderSelectionMenu(dungeonTitle, dungeonOptions);
        const std::size_t dungeonPick = input.getMenuChoice(dungeonOptions.size());

        selectedDungeon = available[dungeonPick];
    }
    else
    {
        selectedDungeon = &DungeonTable::getClassicDungeons().front();
        renderer.renderMessage(
            "Eidolon Labyrinth: no XP earned. Attune available at Rest sites.");
        renderer.presentPause(600);
    }

   
        dungeon.generate(seed, *selectedDungeon, &summonRegistry, runMode);
        const bool won{dungeon.run(playerParty, meta, renderer, input)};

        renderer.renderMessage(won ? "=== RUN COMPLETE ===" : "=== DEFEATED ===");
        renderer.presentPause(1000);
    }
    catch (const QuitException &)
    {
        renderer.clearBattleCache();
        renderer.renderMessage("Saving and exiting...");
        renderer.presentPause(500);
    }

    meta.saveToFile("save.json");
    renderer.renderMessage("Progress saved. Highest floor reached: " + std::to_string(meta.highestFloorReached));
    renderer.presentPause(800);

    return 0;
}