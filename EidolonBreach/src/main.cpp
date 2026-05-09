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

static void togglePartyMember(std::size_t idx, std::vector<bool> &selected,
                              std::size_t &count, std::size_t maxPick)
{
    if (selected[idx])
    {
        selected[idx] = false;
        --count;
    }
    else if (count < maxPick)
    {
        selected[idx] = true;
        ++count;
    }
}

// Returns: 1=confirmed, -1=back/cancel, 0=state changed (re-render needed)
static int processPartyKey(SDL_Keycode key, std::vector<bool> &selected,
                           std::size_t &count, std::size_t &cursor,
                           std::size_t maxPick, std::size_t numAvailable,
                           std::size_t displaySize)
{
    if (key == SDLK_ESCAPE)
        return -1;
    if (key == SDLK_TAB && count > 0)
        return 1;
    if (key == SDLK_UP && cursor > 0)
    {
        --cursor;
        return 0;
    }
    if (key == SDLK_DOWN && cursor + 1 < displaySize)
    {
        ++cursor;
        return 0;
    }
    if (key != SDLK_RETURN && key != SDLK_KP_ENTER)
        return -2; // no-op
    if (cursor == numAvailable && count > 0)
        return 1;
    if (cursor < numAvailable)
    {
        togglePartyMember(cursor, selected, count, maxPick);
        return 0;
    }
    return -2;
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

        bool needsRender = false;
        bool done = false;

        if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat)
        {
            const int r = processPartyKey(event.key.key, selected, selectedCount, cursor,
                                          maxPick, available.size(), display.size());
            if (r == -1)
            {
                party = Party{};
                return;
            }
            if (r == 1)
            {
                done = true;
            }
            else if (r == 0)
                needsRender = true;
        }

        if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_LEFT)
        {
            const float firstRowY = static_cast<float>(renderer.getWindowHeight()) * 0.18f + 36.f;
            const int row = static_cast<int>((event.button.y - firstRowY) / 30.f);
            if (row >= 0 && static_cast<std::size_t>(row) < available.size())
            {
                cursor = static_cast<std::size_t>(row);
                togglePartyMember(cursor, selected, selectedCount, maxPick);
                needsRender = true;
            }
            else if (static_cast<std::size_t>(row) == available.size() && selectedCount > 0)
                done = true;
        }

        if (needsRender)
        {
            display = buildDisplayOptions();
            input.setMenuContext("SELECT PARTY", display);
        }
        if (needsRender || done)
            renderer.renderSelectionMenu("SELECT PARTY", display, cursor);
        if (done)
            break;
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

static void runGame(SDL3Renderer &renderer, SDL3InputHandler &input,
                    const CharacterRegistry &characterRegistry,
                    SummonRegistry &summonRegistry,
                    MetaProgress &meta)
{
    enum class MenuStage
    {
        Mode,
        Dungeon,
        Party,
        Ready
    };
    MenuStage stage{MenuStage::Mode};
    MenuStage partyBackStage{MenuStage::Mode};
    RunMode runMode{RunMode::Classic};
    const DungeonDefinition *selectedDungeon{nullptr};
    Party playerParty{};

    while (stage != MenuStage::Ready)
    {
        switch (stage)
        {
        case MenuStage::Mode:
        {
            const std::vector<std::string> modeOptions{"Classic", "Eidolon Labyrinth"};
            input.setMenuContext("SELECT MODE", modeOptions);
            renderer.renderSelectionMenu("SELECT MODE", modeOptions);
            const std::size_t modeChoice = input.getMenuChoice(modeOptions.size());
            if (modeChoice == IInputHandler::kCancelChoice)
                break;
            runMode = (modeChoice == 1) ? RunMode::EidolonLabyrinth : RunMode::Classic;
            stage = MenuStage::Dungeon;
            break;
        }
        case MenuStage::Dungeon:
        {
            if (runMode == RunMode::Classic)
            {
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

                std::vector<std::string> dungeonOptions{};
                for (const auto *def : available)
                {
                    const int displayFloors{def->fixedLayout.empty()
                                                ? def->numFloors
                                                : static_cast<int>(def->fixedLayout.size())};
                    dungeonOptions.push_back(
                        def->name + "  Rec.Lv." + std::to_string(def->recommendedPlayerLevel) + " | EnemyLv." + std::to_string(def->enemyLevel) + " | " + std::to_string(displayFloors) + " floors" + (meta.clearedDungeonIds.count(def->id) > 0 ? " [CLEARED]" : ""));
                }
                dungeonOptions.push_back("<< Back");

                const std::string dungeonTitle =
                    "DUNGEON SELECT  Player Lv." + std::to_string(meta.playerLevel);
                input.setMenuContext(dungeonTitle, dungeonOptions);
                renderer.renderSelectionMenu(dungeonTitle, dungeonOptions);
                const std::size_t dungeonPick = input.getMenuChoice(dungeonOptions.size());

                if (dungeonPick == IInputHandler::kCancelChoice ||
                    dungeonPick == dungeonOptions.size() - 1)
                {
                    stage = MenuStage::Mode;
                    break;
                }
                selectedDungeon = available[dungeonPick];
                partyBackStage = MenuStage::Dungeon;
            }
            else
            {
                selectedDungeon = &DungeonTable::getClassicDungeons().front();
                renderer.renderMessage(
                    "Eidolon Labyrinth: no XP earned. Attune available at Rest sites.");
                renderer.presentPause(600);
                partyBackStage = MenuStage::Mode;
            }
            stage = MenuStage::Party;
            break;
        }
        case MenuStage::Party:
        {
            playerParty = Party{};
            playerParty.gainSp(50);
            selectParty(playerParty, characterRegistry, meta, renderer, input);
            if (playerParty.size() == 0)
            {
                stage = partyBackStage;
                break;
            }
            stage = MenuStage::Ready;
            break;
        }
        default:
            break;
        }
    }

    renderer.renderMessage("Starting run with " + std::to_string(playerParty.size()) + " character(s).");
    const std::uint32_t seed{static_cast<std::uint32_t>(std::random_device{}())};
    renderer.renderMessage("Run seed: " + std::to_string(seed));
    renderer.presentPause(600);

    Dungeon dungeon{};
    dungeon.generate(seed, *selectedDungeon, &summonRegistry, runMode);
    const bool won{dungeon.run(playerParty, meta, renderer, input)};
    renderer.renderMessage(won ? "=== RUN COMPLETE ===" : "=== DEFEATED ===");
    renderer.presentPause(1000);
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
    SDL3InputHandler input{renderer, renderer};

    // MetaProgress
    MetaProgress meta{MetaProgress::loadFromFile("save.json")};

    // Unlock starting characters if this is a fresh save.
    for (const std::string &id : characterRegistry.getIds())
        std::ignore = meta.unlockCharacter(id);

    try
    {
        runGame(renderer, input, characterRegistry, summonRegistry, meta);
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