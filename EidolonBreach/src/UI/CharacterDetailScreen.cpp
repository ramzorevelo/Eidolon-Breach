/**
 * @file CharacterDetailScreen.cpp
 * @brief CharacterDetailScreen implementation.
 */
#include "UI/CharacterDetailScreen.h"
#include "Characters/CharacterRegistry.h"
#include "Core/CombatConstants.h"
#include "Core/MetaProgress.h"
#include "Core/Stats.h"
#include "UI/AspectTreeScreen.h"
#include "UI/IInputHandler.h"
#include "UI/SDL3InputHandler.h"
#include "UI/SDL3Renderer.h"
#include <string>
#include <vector>

void CharacterDetailScreen::showStatsTab(std::string_view characterId,
                                         const MetaProgress &meta,
                                         const CharacterRegistry &registry,
                                         SDL3Renderer &renderer,
                                         SDL3InputHandler &input)
{
    const std::string id{characterId};
    const int level{meta.characterLevels.count(id) ? meta.characterLevels.at(id) : 1};

    // base = no mods applied; withMods = CharacterMod + Echo effects applied.
    const auto base{registry.create(id, level)};
    const auto withMods{registry.create(id, level, &meta)};

    std::vector<std::string> opts{};

    if (base && withMods)
    {
        const Stats &b{base->getBaseStats()};
        const Stats &m{withMods->getFinalStats()};
        const CharacterInsightData &insight{
            meta.characterInsight.count(id)
                ? meta.characterInsight.at(id)
                : CharacterInsightData{}};

        opts.push_back("Name:      " + base->getName());
        opts.push_back("Archetype: " + registry.getArchetype(id));
        opts.push_back("Level:     " + std::to_string(level));
        opts.push_back("HP:        " + std::to_string(b.maxHp) + " -> " + std::to_string(m.maxHp));
        opts.push_back("ATK:       " + std::to_string(b.atk) + " -> " + std::to_string(m.atk));
        opts.push_back("DEF:       " + std::to_string(b.def) + " -> " + std::to_string(m.def));
        opts.push_back("SPD:       " + std::to_string(b.spd) + " -> " + std::to_string(m.spd));
        opts.push_back("Echo:      " + std::to_string(insight.echoCount) +
                       " / " + std::to_string(CombatConstants::kMaxEchoes));
        opts.push_back("Insight:   " + std::to_string(insight.insightBalance));
    }
    else
    {
        opts.push_back("(character data unavailable)");
    }

    opts.push_back("<< Back");
    const std::string title{"STATS -- " + id};
    input.setMenuContext(title, opts);
    renderer.renderSelectionMenu(title, opts);
    input.getMenuChoice(opts.size());
}

void CharacterDetailScreen::showEquipmentTab(std::string_view characterId,
                                             SDL3Renderer &renderer,
                                             SDL3InputHandler &input)
{
    // Persistent inventory is not yet wired at the hub level.
    const std::vector<std::string> opts{
        "No equipment equipped.",
        "<< Back",
    };
    const std::string title{"EQUIPMENT -- " + std::string{characterId}};
    input.setMenuContext(title, opts);
    renderer.renderSelectionMenu(title, opts);
    input.getMenuChoice(opts.size());
}

void CharacterDetailScreen::run(std::string_view characterId,
                                MetaProgress &meta,
                                const CharacterRegistry &registry,
                                SDL3Renderer &renderer,
                                SDL3InputHandler &input)
{
    while (true)
    {
        const std::vector<std::string> tabs{"Stats", "Equipment", "Aspect Tree", "<< Back"};
        const std::string title{"CHARACTER -- " + std::string{characterId}};
        input.setMenuContext(title, tabs);
        renderer.renderSelectionMenu(title, tabs);
        const std::size_t pick{input.getMenuChoice(tabs.size())};

        if (pick == IInputHandler::kCancelChoice || pick == 3)
            return;
        if (pick == 0)
            showStatsTab(characterId, meta, registry, renderer, input);
        else if (pick == 1)
            showEquipmentTab(characterId, renderer, input);
        else if (pick == 2)
            AspectTreeScreen::run(characterId, meta, registry, renderer, input);
    }
}