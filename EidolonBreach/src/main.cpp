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
 * @brief Multi-select character screen. Player toggles characters with Enter,
 *        confirms the full party with Tab. Auto-selects when roster <= kMaxPlayerCharacters.
 */
static void selectParty(Party &party,
                        const CharacterRegistry &characterRegistry,
                        const MetaProgress &meta,
                        SDL3Renderer &renderer,
                        SDL3InputHandler &input)
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

    const std::size_t maxPick = static_cast<std::size_t>(CombatConstants::kMaxPlayerCharacters);

    if (available.size() <= maxPick)
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

    // Build display options showing stats per character.
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

    std::vector<bool> selected(available.size(), false);
    std::size_t selectedCount{0};
    std::size_t cursor{0};

    auto buildDisplayOptions = [&]() -> std::vector<std::string>
    {
        std::vector<std::string> display{};
        for (std::size_t i = 0; i < options.size(); ++i)
        {
            const std::string prefix = selected[i] ? "[X] " : "[ ] ";
            display.push_back(prefix + options[i]);
        }
        display.push_back("--- Confirm Party (" + std::to_string(selectedCount) + "/" + std::to_string(maxPick) + ") ---");
        return display;
    };

    auto display = buildDisplayOptions();
    input.setMenuContext("SELECT PARTY", display);
    renderer.renderSelectionMenu("SELECT PARTY", display, cursor);

    SDL_Event event{};
    while (SDL_WaitEvent(&event))
    {
        if (event.type == SDL_EVENT_QUIT)
            throw QuitException{};

        if (event.type == SDL_EVENT_KEY_DOWN)
        {
            if (event.key.key == SDLK_UP && cursor > 0)
            {
                --cursor;
                renderer.renderSelectionMenu("SELECT PARTY", display, cursor);
            }
            else if (event.key.key == SDLK_DOWN && cursor + 1 < display.size())
            {
                ++cursor;
                renderer.renderSelectionMenu("SELECT PARTY", display, cursor);
            }
            else if (event.key.key == SDLK_RETURN || event.key.key == SDLK_KP_ENTER)
            {
                if (cursor < available.size())
                {
                    // Toggle character selection.
                    if (selected[cursor])
                    {
                        selected[cursor] = false;
                        --selectedCount;
                    }
                    else if (selectedCount < maxPick)
                    {
                        selected[cursor] = true;
                        ++selectedCount;
                    }
                    display = buildDisplayOptions();
                    input.setMenuContext("SELECT PARTY", display);
                    renderer.renderSelectionMenu("SELECT PARTY", display, cursor);
                }
                else if (cursor == available.size() && selectedCount > 0)
                {
                    break; // Confirm selection.
                }
            }
            else if (event.key.key == SDLK_TAB && selectedCount > 0)
            {
                break; // Tab confirms from anywhere.
            }
        }

        if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
            event.button.button == SDL_BUTTON_LEFT)
        {
            const float firstRowY = static_cast<float>(renderer.getWindowHeight()) * 0.18f + 36.f;
            const float rowH = 30.f;
            const float fy = static_cast<float>(event.button.y);

            if (fy >= firstRowY)
            {
                const int row = static_cast<int>((fy - firstRowY) / rowH);
                if (row >= 0 && static_cast<std::size_t>(row) < available.size())
                {
                    cursor = static_cast<std::size_t>(row);
                    if (selected[cursor])
                    {
                        selected[cursor] = false;
                        --selectedCount;
                    }
                    else if (selectedCount < maxPick)
                    {
                        selected[cursor] = true;
                        ++selectedCount;
                    }
                    display = buildDisplayOptions();
                    input.setMenuContext("SELECT PARTY", display);
                    renderer.renderSelectionMenu("SELECT PARTY", display, cursor);
                }
                else if (static_cast<std::size_t>(row) == available.size() && selectedCount > 0)
                {
                    break;
                }
            }
        }
    }

    for (std::size_t i = 0; i < available.size(); ++i)
    {
        if (!selected[i])
            continue;
        const int level{meta.characterLevels.count(available[i]) > 0
                            ? meta.characterLevels.at(available[i])
                            : 1};
        auto pc{characterRegistry.create(available[i], level)};
        if (pc)
            party.addUnit(std::move(pc));
    }
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