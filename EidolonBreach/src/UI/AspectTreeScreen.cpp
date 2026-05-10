/**
 * @file AspectTreeScreen.cpp
 * @brief AspectTreeScreen implementation.
 */
#include "UI/AspectTreeScreen.h"
#include "Characters/CharacterRegistry.h"
#include "Core/MetaProgress.h"
#include "UI/SDL3InputHandler.h"
#include "UI/SDL3Renderer.h"
#include <algorithm>
#include <string>

namespace
{
std::string signalLabel(BehaviorSignal s)
{
    switch (s)
    {
    case BehaviorSignal::Aggressive:
        return "Aggressive";
    case BehaviorSignal::Methodical:
        return "Methodical";
    case BehaviorSignal::Sacrificial:
        return "Sacrificial";
    case BehaviorSignal::Supportive:
        return "Supportive";
    case BehaviorSignal::Reactive:
        return "Reactive";
    }
    return "";
}

std::string nodeStateLabel(const AspectTreeNode &node,
                           const MetaProgress &meta,
                           std::string_view characterId)
{
    const auto it{meta.characterInsight.find(std::string{characterId})};
    if (it == meta.characterInsight.end())
        return "[LOCKED]";

    for (const auto &chosen : it->second.chosenAspects)
        if (chosen == node.id)
            return "[ACTIVE]";

    const auto tallyIt{it->second.signalTallies.find(node.branch)};
    const int tally{(tallyIt != it->second.signalTallies.end()) ? tallyIt->second : 0};
    if (tally >= node.unlockThreshold)
        return "[UNLOCKED]";

    return "[LOCKED " + std::to_string(tally) + "/" +
           std::to_string(node.unlockThreshold) + "]";
}
} // namespace

void AspectTreeScreen::run(std::string_view characterId,
                           MetaProgress &meta,
                           const CharacterRegistry & /*registry*/,
                           SDL3Renderer &renderer,
                           SDL3InputHandler &input)
{
    const AspectTree tree{AspectTree::loadForCharacter(characterId)};
    std::size_t selectedIndex{0};

    while (true)
    {
        drawTree(tree, meta, characterId, selectedIndex, renderer);

        const std::vector<std::size_t> selectable{
            buildSelectableIndices(tree, meta, characterId)};

        // Build option list for getMenuChoice: navigable nodes + Back.
        std::vector<std::string> options{};
        for (std::size_t idx : selectable)
        {
            const auto &node{tree.nodes()[idx]};
            options.push_back(
                signalLabel(node.branch) + " | " + node.id + "  " +
                nodeStateLabel(node, meta, characterId) +
                "  Cost: " + std::to_string(node.insightCost));
        }
        options.push_back("Back");

        const std::string title{std::string{characterId} + " — Aspect Tree"};
        input.setMenuContext(title, options);
        renderer.renderSelectionMenu(title, options, selectedIndex);
        const std::size_t pick{input.getMenuChoice(options.size())};

        if (pick >= options.size() - 1)
            return;

        selectedIndex = pick;

        if (pick < selectable.size())
        {
            const auto &node{tree.nodes()[selectable[pick]]};
            const bool activated{meta.spendInsight(characterId, node.id, tree)};
            if (!activated)
                renderer.renderMessage("Cannot activate: check branch tally and Insight balance.");
        }
    }
}

void AspectTreeScreen::drawTree(const AspectTree &tree,
                                const MetaProgress &meta,
                                std::string_view characterId,
                                std::size_t /*selectedIndex*/,
                                SDL3Renderer &renderer)
{
    // Gather Insight balance line.
    int balance{0};
    const auto it{meta.characterInsight.find(std::string{characterId})};
    if (it != meta.characterInsight.end())
        balance = it->second.insightBalance;

    renderer.renderMessage("Insight: " + std::to_string(balance));
    for (const auto &node : tree.nodes())
        renderer.renderMessage("  " + signalLabel(node.branch) + " | " +
                               node.id + "  " +
                               nodeStateLabel(node, meta, characterId));
}

std::vector<std::size_t>
AspectTreeScreen::buildSelectableIndices(const AspectTree &tree,
                                         const MetaProgress & /*meta*/,
                                         std::string_view /*characterId*/)
{
    std::vector<std::size_t> result{};
    for (std::size_t i{0}; i < tree.nodes().size(); ++i)
        result.push_back(i);
    return result;
}