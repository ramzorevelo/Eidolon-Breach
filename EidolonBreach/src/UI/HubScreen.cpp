/**
 * @file HubScreen.cpp
 * @brief HubScreen implementation.
 *        selectParty, togglePartyMember, processPartyKey, and dungeon helpers
 *        moved verbatim from main.cpp.
 */
#include "UI/HubScreen.h"
#include "Characters/AbilityRegistry.h"
#include "Characters/CharacterRegistry.h"
#include "Core/CombatConstants.h"
#include "Core/MetaProgress.h"
#include "Core/RunContext.h"
#include "Core/Stats.h"
#include "Dungeon/Dungeon.h"
#include "Dungeon/DungeonDefinition.h"
#include "Dungeon/DungeonTable.h"
#include "Entities/Party.h"
#include "Items/Inventory.h"
#include "Meta/BondTrial.h"
#include "Summons/SummonRegistry.h"
#include "UI/BackpackScreen.h"
#include "UI/CharacterDetailScreen.h"
#include "UI/IInputHandler.h"
#include "UI/IRenderer.h"
#include "UI/SDL3InputHandler.h"
#include "UI/SDL3Renderer.h"
#include "UI/SettingsScreen.h"
#include <SDL3/SDL.h>
#include <algorithm>
#include <cstdint>
#include <random>
#include <string>
#include <vector>

namespace
{
std::string_view chapterBossId(std::string_view dungeonId)
{
    if (dungeonId == "dungeon_01" || dungeonId == "dungeon_02" ||
        dungeonId == "dungeon_03")
        return "dungeon_03";
    if (dungeonId == "dungeon_04" || dungeonId == "dungeon_05" ||
        dungeonId == "dungeon_06")
        return "dungeon_06";
    if (dungeonId == "dungeon_07" || dungeonId == "dungeon_08" ||
        dungeonId == "dungeon_09")
        return "dungeon_09";
    return "dungeon_10";
}

bool isHardAvailable(const DungeonDefinition &def, const MetaProgress &meta)
{
    return meta.clearedDungeonIds.count(std::string{chapterBossId(def.id)}) > 0;
}

bool isNightmareAvailable(const DungeonDefinition &def, const MetaProgress &meta)
{
    return meta.hardClearedDungeonIds.count(std::string{chapterBossId(def.id)}) > 0;
}

void togglePartyMember(std::size_t idx,
                       std::vector<bool> &selected,
                       std::vector<std::size_t> &selectionOrder,
                       std::size_t &count,
                       std::size_t maxPick)
{
    if (selected[idx])
    {
        selected[idx] = false;
        selectionOrder.erase(std::find(selectionOrder.begin(), selectionOrder.end(), idx));
        --count;
    }
    else if (count < maxPick)
    {
        selected[idx] = true;
        selectionOrder.push_back(idx);
        ++count;
    }
}

// Returns: 1=confirmed, -1=back/cancel, 0=state changed, -2=no-op.
int processPartyKey(SDL_Keycode key,
                    std::vector<bool> &selected,
                    std::vector<std::size_t> &selectionOrder,
                    std::size_t &count,
                    std::size_t &cursor,
                    std::size_t maxPick,
                    std::size_t numAvailable,
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
        return -2;
    if (cursor == numAvailable && count > 0)
        return 1;
    if (cursor < numAvailable)
    {
        togglePartyMember(cursor, selected, selectionOrder, count, maxPick);
        return 0;
    }
    return -2;
}

void selectParty(Party &party,
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

    const std::size_t maxPick{static_cast<std::size_t>(CombatConstants::kMaxPlayerCharacters)};

    std::vector<std::string> options{};
    for (std::size_t i{0}; i < available.size(); ++i)
    {
        const auto pc{characterRegistry.create(available[i])};
        if (!pc)
            continue;
        const int level{meta.characterLevels.count(available[i]) > 0
                            ? meta.characterLevels.at(available[i])
                            : 1};
        const Stats &s{pc->getBaseStats()};
        options.push_back(pc->getName() + " [" + characterRegistry.getArchetype(available[i]) + "]" +
                          "  Lv." + std::to_string(level) +
                          "  HP:" + std::to_string(s.maxHp) +
                          " ATK:" + std::to_string(s.atk) +
                          " SPD:" + std::to_string(s.spd));
    }

    std::vector<bool> selected(available.size(), false);
    std::size_t selectedCount{0};
    std::vector<std::size_t> selectionOrder{};
    std::size_t cursor{0};

    auto buildDisplayOptions{[&]() -> std::vector<std::string>
                             {
                                 std::vector<std::string> display{};
                                 for (std::size_t i{0}; i < options.size(); ++i)
                                 {
                                     std::string prefix{};
                                     if (!selected[i])
                                     {
                                         prefix = "[ ] ";
                                     }
                                     else
                                     {
                                         const auto it{std::find(selectionOrder.begin(), selectionOrder.end(), i)};
                                         const std::size_t pos{static_cast<std::size_t>(it - selectionOrder.begin()) + 1};
                                         prefix = "[" + std::to_string(pos) + "] ";
                                     }
                                     display.push_back(prefix + options[i]);
                                 }
                                 display.push_back("--- Confirm Party (" + std::to_string(selectedCount) +
                                                   "/" + std::to_string(maxPick) + ") ---");
                                 return display;
                             }};

    auto display{buildDisplayOptions()};
    input.setMenuContext("SELECT PARTY", display);
    renderer.renderSelectionMenu("SELECT PARTY", display, cursor);

    SDL_Event event{};
    while (SDL_WaitEvent(&event))
    {
        if (event.type == SDL_EVENT_QUIT)
            throw QuitException{};

        bool needsRender{false};
        bool done{false};

        if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat)
        {
            const int r{processPartyKey(event.key.key, selected, selectionOrder,
                                        selectedCount, cursor, maxPick,
                                        available.size(), display.size())};
            if (r == -1)
            {
                party = Party{};
                return;
            }
            if (r == 1)
                done = true;
            else if (r == 0)
                needsRender = true;
        }

        if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_LEFT)
        {
            const float firstRowY{static_cast<float>(renderer.getWindowHeight()) * 0.18f + 36.f};
            const int row{static_cast<int>((event.button.y - firstRowY) / 30.f)};
            if (row >= 0 && static_cast<std::size_t>(row) < available.size())
            {
                cursor = static_cast<std::size_t>(row);
                togglePartyMember(cursor, selected, selectionOrder, selectedCount, maxPick);
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

    for (const std::size_t i : selectionOrder)
    {
        const int level{meta.characterLevels.count(available[i]) > 0
                            ? meta.characterLevels.at(available[i])
                            : 1};
        auto pc{characterRegistry.create(available[i], level)};
        if (pc)
            party.addUnit(std::move(pc));
    }
}
} // namespace

HubScreen::HubScreen(SDL3Renderer &renderer,
                     SDL3InputHandler &input,
                     CharacterRegistry &characterRegistry,
                     const AbilityRegistry &abilityRegistry,
                     SummonRegistry &summonRegistry,
                     MetaProgress &meta,
                     Settings &settings)
    : m_renderer{renderer}, m_input{input}, m_characterRegistry{characterRegistry}, m_abilityRegistry{abilityRegistry}, m_summonRegistry{summonRegistry}, m_meta{meta}, m_settings{settings}
{
}

void HubScreen::run()
{
    while (true)
    {
        const std::vector<std::string> opts{
            "Characters",
            "Backpack",
            "Classic",
            "Eidolon Labyrinth",
            "Bond Trials",
            "Settings",
            "Exit Game",
        };
        m_input.setMenuContext("EIDOLON BREACH", opts);
        m_renderer.renderSelectionMenu("EIDOLON BREACH", opts);
        const std::size_t pick{m_input.getMenuChoice(opts.size())};

        if (pick == IInputHandler::kCancelChoice)
            continue;

        switch (pick)
        {
        case 0:
            showCharacters();
            break;
        case 1:
            showBackpack();
            break;
        case 2:
            launchClassic();
            break;
        case 3:
            launchLabyrinth();
            break;
        case 4:
            showBondTrials();
            break;
        case 5:
            showSettings();
            break;
        case 6:
            exitGame();
            break;
        default:
            break;
        }
    }
}

void HubScreen::showCharacters()
{
    const std::vector<std::string> &allIds{m_characterRegistry.getIds()};
    std::vector<std::string> unlockedIds{};
    for (const auto &id : allIds)
        if (m_meta.unlockedCharacterIds.count(id) > 0)
            unlockedIds.push_back(id);

    std::vector<std::string> opts{unlockedIds};
    opts.push_back("<< Back");

    while (true)
    {
        m_input.setMenuContext("CHARACTERS", opts);
        m_renderer.renderSelectionMenu("CHARACTERS", opts);
        const std::size_t pick{m_input.getMenuChoice(opts.size())};

        if (pick == IInputHandler::kCancelChoice || pick == opts.size() - 1)
            return;
        CharacterDetailScreen::run(unlockedIds[pick], m_meta, m_characterRegistry,
                                   m_renderer, m_input);
    }
}

void HubScreen::showBackpack()
{
    // Persistent inventory wiring is deferred to v0.9.8; display an empty one.
    const Inventory empty{};
    BackpackScreen::run(empty, m_renderer, m_input);
}

void HubScreen::launchClassic()
{
    const auto &classicDungeons{DungeonTable::getClassicDungeons()};

    while (true)
    {
        // Rebuild each iteration so a newly-cleared dungeon updates [CLEARED] immediately.
        std::vector<const DungeonDefinition *> available{};
        bool previousCleared{true};
        for (const auto &def : classicDungeons)
        {
            if (previousCleared)
                available.push_back(&def);
            previousCleared = m_meta.clearedDungeonIds.count(def.id) > 0;
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
                def->name + "  Rec.Lv." + std::to_string(def->recommendedPlayerLevel) +
                " | EnemyLv." + std::to_string(def->enemyLevel) +
                " | " + std::to_string(displayFloors) + " floors" +
                (m_meta.clearedDungeonIds.count(def->id) > 0 ? " [CLEARED]" : "") +
                (isHardAvailable(*def, m_meta)
                     ? (isNightmareAvailable(*def, m_meta) ? " [N]" : " [H]")
                     : ""));
        }
        dungeonOptions.push_back("<< Back");

        const std::string dungeonTitle{
            "DUNGEON SELECT  Player Lv." + std::to_string(m_meta.playerLevel)};

        std::vector<DungeonSelectInfo> dungeonInfos{};
        for (const auto *def : available)
        {
            DungeonSelectInfo info{};
            info.name = def->name;
            info.description = def->description;
            info.recommendedLevel = def->recommendedPlayerLevel;
            info.enemyLevel = def->enemyLevel;
            info.layout = def->fixedLayout.empty()
                              ? std::vector<std::string>(
                                    static_cast<std::size_t>(def->numFloors), "battle")
                              : def->fixedLayout;
            info.cleared = m_meta.clearedDungeonIds.count(def->id) > 0;
            info.difficultyLabel =
                isNightmareAvailable(*def, m_meta) ? "Nightmare"
                : isHardAvailable(*def, m_meta)    ? "Hard"
                                                   : "Normal";
            dungeonInfos.push_back(std::move(info));
        }

        m_input.setMenuContext(dungeonTitle, dungeonOptions);
        m_renderer.renderDungeonSelect(dungeonTitle, dungeonInfos);
        const std::size_t dungeonPick{m_input.getMenuChoice(dungeonOptions.size())};

        if (dungeonPick == IInputHandler::kCancelChoice ||
            dungeonPick == dungeonOptions.size() - 1)
            return;

        const DungeonDefinition *selectedDungeon{available[dungeonPick]};

        DungeonDefinition adjustedDef{};
        {
            std::vector<std::string> diffOpts{"Normal"};
            if (isHardAvailable(*selectedDungeon, m_meta))
                diffOpts.push_back("Hard");
            if (isNightmareAvailable(*selectedDungeon, m_meta))
                diffOpts.push_back("Nightmare");
            diffOpts.push_back("<< Back");

            m_input.setMenuContext("SELECT DIFFICULTY", diffOpts);
            m_renderer.renderSelectionMenu("SELECT DIFFICULTY", diffOpts);
            const std::size_t diffPick{m_input.getMenuChoice(diffOpts.size())};

            if (diffPick == IInputHandler::kCancelChoice ||
                diffPick == diffOpts.size() - 1)
                continue;

            adjustedDef = *selectedDungeon;
            if (diffOpts[diffPick] == "Hard")
                adjustedDef.difficulty = DungeonDifficulty::Hard;
            else if (diffOpts[diffPick] == "Nightmare")
                adjustedDef.difficulty = DungeonDifficulty::Nightmare;
            else
                adjustedDef.difficulty = DungeonDifficulty::Normal;
            selectedDungeon = &adjustedDef;
        }

        Party playerParty{};
        playerParty.gainSp(50);
        selectParty(playerParty, m_characterRegistry, m_meta, m_renderer, m_input);
        if (playerParty.size() == 0)
            continue;

        m_renderer.renderMessage("Starting run with " +
                                 std::to_string(playerParty.size()) + " character(s).");
        const std::uint32_t seed{static_cast<std::uint32_t>(std::random_device{}())};
        m_renderer.renderMessage("Run seed: " + std::to_string(seed));
        m_renderer.presentPause(600);

        Dungeon dungeon{};
        dungeon.generate(seed, *selectedDungeon, &m_summonRegistry, RunMode::Classic);
        const bool won{dungeon.run(playerParty, m_meta, m_renderer, m_input)};
        m_renderer.renderMessage(won ? "=== RUN COMPLETE ===" : "=== DEFEATED ===");
        m_renderer.presentPause(1000);
    }
}

void HubScreen::launchLabyrinth()
{
    const std::vector<std::string> opts{"(Coming Soon)", "<< Back"};
    m_input.setMenuContext("EIDOLON LABYRINTH", opts);
    m_renderer.renderSelectionMenu("EIDOLON LABYRINTH", opts);
    m_input.getMenuChoice(opts.size());
}

void HubScreen::showBondTrials()
{
    constexpr int kBondTrialRunGate{5};

    const std::vector<std::string> &allIds{m_characterRegistry.getIds()};
    std::vector<std::string> opts{};
    std::vector<std::string> eligibleIds{};

    for (const auto &id : allIds)
    {
        if (m_meta.unlockedCharacterIds.count(id) == 0)
            continue;
        const CharacterInsightData &insight{
            m_meta.characterInsight.count(id)
                ? m_meta.characterInsight.at(id)
                : CharacterInsightData{}};
        const int runs{m_meta.characterRunCounts.count(id)
                           ? m_meta.characterRunCounts.at(id)
                           : 0};

        if (insight.bondTrialComplete)
        {
            opts.push_back(id + "  [COMPLETE]");
            eligibleIds.push_back("");
        }
        else if (runs >= kBondTrialRunGate)
        {
            opts.push_back(id);
            eligibleIds.push_back(id);
        }
        else
        {
            opts.push_back(id + "  [" + std::to_string(runs) + "/" +
                           std::to_string(kBondTrialRunGate) + " runs needed]");
            eligibleIds.push_back("");
        }
    }

    opts.push_back("<< Back");
    eligibleIds.push_back("");

    while (true)
    {
        m_input.setMenuContext("BOND TRIALS", opts);
        m_renderer.renderSelectionMenu("BOND TRIALS", opts);
        const std::size_t pick{m_input.getMenuChoice(opts.size())};

        if (pick == IInputHandler::kCancelChoice || pick == opts.size() - 1)
            return;
        if (pick < eligibleIds.size() && !eligibleIds[pick].empty())
            BondTrial::run(eligibleIds[pick], m_meta, m_characterRegistry,
                           m_abilityRegistry, m_summonRegistry, m_renderer, m_input);
    }
}

void HubScreen::showSettings()
{
    SettingsScreen::run(m_settings, m_renderer, m_input);
}

void HubScreen::exitGame()
{
    const std::vector<std::string> opts{"Yes, Exit", "No, Stay"};
    m_input.setMenuContext("EXIT GAME?", opts);
    m_renderer.renderSelectionMenu("EXIT GAME?", opts);
    const std::size_t pick{m_input.getMenuChoice(opts.size())};
    if (pick == 0)
        throw QuitException{};
}