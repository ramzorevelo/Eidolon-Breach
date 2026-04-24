/**
 * @file Dungeon.cpp
 * @brief Graph-based dungeon generation and run-loop.
 */

#include "Dungeon/Dungeon.h"
#include "Core/BattleEvents.h"
#include "Core/CombatConstants.h"
#include "Core/MetaProgress.h"
#include "Dungeon/BattleNode.h"
#include "Dungeon/BossNode.h"
#include "Dungeon/EliteNode.h"
#include "Dungeon/EventNode.h"
#include "Dungeon/RestNode.h"
#include "Dungeon/TreasureNode.h"
#include "Entities/Party.h"
#include "Entities/Slime.h"
#include "Entities/StoneGolem.h"
#include "Entities/VampireBat.h"
#include <algorithm>
#include <iostream>
#include <limits>
#include <random>

namespace
{
void populateSlimeParty(Party &p)
{
    p.addUnit(std::make_unique<Slime>("Slime Alpha", 80, 30));
    p.addUnit(std::make_unique<Slime>("Slime Beta", 60, 25));
}
void populateGolemParty(Party &p)
{
    p.addUnit(std::make_unique<StoneGolem>("Stone Golem", 130, 60));
}
void populateBatParty(Party &p)
{
    p.addUnit(std::make_unique<VampireBat>("Vampire Bat", 100, 40));
}
void populateMixedParty(Party &p)
{
    p.addUnit(std::make_unique<Slime>("Slime", 70, 25));
    p.addUnit(std::make_unique<VampireBat>("Vampire Bat", 90, 35));
}
void populateBossParty(Party &p)
{
    p.addUnit(std::make_unique<StoneGolem>("Breach Warden", 200, 90));
    p.addUnit(std::make_unique<VampireBat>("Shard Wraith", 120, 50));
}

using EnemyFactory = std::function<void(Party &)>;

EnemyFactory pickEnemyFactory(std::mt19937 &rng)
{
    std::uniform_int_distribution<int> dist{0, 2};
    switch (dist(rng))
    {
    case 0:
        return populateSlimeParty;
    case 1:
        return populateBatParty;
    default:
        return populateMixedParty;
    }
}

EnemyFactory pickEliteFactory(std::mt19937 &rng)
{
    std::uniform_int_distribution<int> dist{0, 1};
    return dist(rng) == 0 ? populateGolemParty : populateBatParty;
}

/**
 * @brief Elite weight table per difficulty.
 *        Weight is out of 10; remaining weight is split between battle and rest/treasure.
 */
int eliteWeight(int layer, int numLayers, DungeonDifficulty difficulty)
{
    const float depthRatio{static_cast<float>(layer) / static_cast<float>(numLayers)};
    int base{0};
    switch (difficulty)
    {
    case DungeonDifficulty::Normal:
        base = 1;
        break;
    case DungeonDifficulty::Hard:
        base = 3;
        break;
    case DungeonDifficulty::Nightmare:
        base = 5;
        break;
    }
    // Weight grows with depth: deeper floors are more likely to have elites.
    return base + static_cast<int>(depthRatio * 3.0f);
}
} // namespace

void Dungeon::generate(std::uint32_t seed,
                       int numLayers,
                       DungeonDifficulty difficulty)
{
    m_difficulty = difficulty;
    m_runContext.reset();
    m_layers.clear();
    assignFloorAffinities(seed, numLayers);
    buildGraph(seed, numLayers, difficulty);
}

void Dungeon::assignFloorAffinities(std::uint32_t seed, int numLayers)
{
    std::mt19937 rng{seed + 1u};
    std::uniform_int_distribution<int> dist{0, 4};
    m_floorAffinities.resize(static_cast<std::size_t>(numLayers));
    for (auto &a : m_floorAffinities)
        a = static_cast<Affinity>(dist(rng));
}

std::unique_ptr<MapNode> Dungeon::makeNode(int layer,
                                           int numLayers,
                                           DungeonDifficulty difficulty,
                                           std::mt19937 &rng,
                                           bool noElite,
                                           bool noRest,
                                           bool noTreasure) const
{
    const Affinity floorAffinity{
        m_floorAffinities[static_cast<std::size_t>(layer)]};
    const int gold{20 + layer * 5};

    const int eW{noElite ? 0 : eliteWeight(layer, numLayers, difficulty)};
    const int battleW{10 - eW / 2};
    const int restW{noRest ? 0 : 3};
    const int treasureW{noTreasure ? 0 : 2};
    const int totalW{eW + battleW + restW + treasureW};

    std::uniform_int_distribution<int> dist{0, totalW - 1};
    const int roll{dist(rng)};

    if (roll < eW)
        return std::make_unique<EliteNode>(pickEliteFactory(rng), floorAffinity);
    if (roll < eW + battleW)
        return std::make_unique<BattleNode>(pickEnemyFactory(rng), floorAffinity);
    if (roll < eW + battleW + restW)
        return std::make_unique<RestNode>();
    return std::make_unique<TreasureNode>(gold);
}

void Dungeon::buildGraph(std::uint32_t seed,
                         int numLayers,
                         DungeonDifficulty difficulty)
{
    std::mt19937 rng{seed};
    std::uniform_int_distribution<int> widthDist{1, 3};

    bool prevLayerHadElite{false};
    bool prevLayerHadRest{false};
    bool prevLayerHadTreasure{false};

    for (int layer{0}; layer < numLayers; ++layer)
    {
        const Affinity floorAffinity{
            m_floorAffinities[static_cast<std::size_t>(layer)]};

        std::vector<DungeonGraphNode> layerNodes{};
        const bool isBossFloor{layer == numLayers - 1};
        const bool isPreBossFloor{layer == numLayers - 2};
        const bool isEliteGateFloor{layer == numLayers - 3};

        if (isBossFloor)
        {
            layerNodes.push_back({std::make_unique<BossNode>(
                                      populateBossParty, floorAffinity),
                                  {}});
            prevLayerHadElite = false;
            prevLayerHadRest = false;
            prevLayerHadTreasure = false;
        }
        else if (isPreBossFloor)
        {
            // Always a single Rest node. Gives the player guaranteed recovery
            // before the boss regardless of prior path.
            layerNodes.push_back({std::make_unique<RestNode>(), {}});
            prevLayerHadElite = false;
            prevLayerHadRest = true;
            prevLayerHadTreasure = false;
        }
        else if (isEliteGateFloor)
        {
            layerNodes.push_back({std::make_unique<EliteNode>(
                                      pickEliteFactory(rng), floorAffinity),
                                  {}});
            prevLayerHadElite = true;
            prevLayerHadRest = false;
            prevLayerHadTreasure = false;
        }
        else
        {
            const int width{widthDist(rng)};
            bool thisLayerHadElite{false};
            bool thisLayerHadRest{false};
            bool thisLayerHadTreasure{false};
            const bool isPreEliteGate{layer == numLayers - 4};
            for (int i{0}; i < width; ++i)
            {
                auto node{makeNode(layer, numLayers, difficulty, rng,
                                   prevLayerHadElite || isPreEliteGate,
                                   prevLayerHadRest,
                                   prevLayerHadTreasure)};
                if (dynamic_cast<EliteNode *>(node.get()))
                    thisLayerHadElite = true;
                if (dynamic_cast<RestNode *>(node.get()))
                    thisLayerHadRest = true;
                if (dynamic_cast<TreasureNode *>(node.get()))
                    thisLayerHadTreasure = true;
                layerNodes.push_back({std::move(node), {}});
            }
            prevLayerHadElite = thisLayerHadElite;
            prevLayerHadRest = thisLayerHadRest;
            prevLayerHadTreasure = thisLayerHadTreasure;
        }

        m_layers.push_back(std::move(layerNodes));
    }

    // Connect layers: each node connects to 1-2 nodes in the next layer.
    for (int layer{0}; layer < static_cast<int>(m_layers.size()) - 1; ++layer)
        connectLayers(layer, rng);
}

void Dungeon::connectLayers(int layerIndex, std::mt19937 &rng)
{
    auto &current{m_layers[static_cast<std::size_t>(layerIndex)]};
    const auto &next{m_layers[static_cast<std::size_t>(layerIndex + 1)]};
    const int nextSize{static_cast<int>(next.size())};

    for (int i{0}; i < static_cast<int>(current.size()); ++i)
    {
        // Each node connects to at least one next node, possibly two.
        const int primary{std::min(i, nextSize - 1)};
        current[static_cast<std::size_t>(i)].nextIndices.push_back(primary);

        // 40% chance of a second connection to an adjacent next node.
        std::uniform_int_distribution<int> chanceDist{0, 9};
        if (nextSize > 1 && chanceDist(rng) < 4)
        {
            const int secondary{primary == 0 ? 1 : primary - 1};
            current[static_cast<std::size_t>(i)].nextIndices.push_back(secondary);
        }
    }

    // Guarantee every next-layer node is reachable from at least one current node.
    for (int j{0}; j < nextSize; ++j)
    {
        bool reachable{false};
        for (const auto &node : current){
            for (int idx : node.nextIndices)
            {

                if (idx == j)
                {
                    reachable = true;
                    break;
                }
            }
    }
        if (!reachable)
        {
            auto &indices{current[0].nextIndices};
            if (std::find(indices.begin(), indices.end(), j) == indices.end())
                indices.push_back(j);
        }
    }
}

std::vector<int> Dungeon::getReachableIndices(
    int layerIndex,
    const std::vector<int> &currentIndices) const
{
    std::vector<int> reachable{};
    for (int idx : currentIndices)
    {
        const auto &node{m_layers[static_cast<std::size_t>(layerIndex)]
                                 [static_cast<std::size_t>(idx)]};
        for (int next : node.nextIndices)
        {
            if (std::find(reachable.begin(), reachable.end(), next) == reachable.end())
                reachable.push_back(next);
        }
    }
    return reachable;
}

bool Dungeon::run(Party &party, MetaProgress &meta)
{
    int floorsCleared{0};

    // Player may start from any node on floor 0.
    std::vector<int> reachable{};
    for (int i{0}; i < static_cast<int>(m_layers[0].size()); ++i)
        reachable.push_back(i);

    for (int layer{0}; layer < static_cast<int>(m_layers.size()); ++layer)
    {
        if (party.isAllDead())
            break;

        std::cout << "\n--- Floor " << (layer + 1)
                  << "/" << m_layers.size()
                  << " | Affinity: "
                  << affinityToString(m_floorAffinities[static_cast<std::size_t>(layer)])
                  << " ---\n";

        const int chosen{presentChoices(party, meta, layer, reachable)};
        if (chosen == -1)
        {
            break;
        }

        ++floorsCleared;
        meta.highestFloorReached = std::max(meta.highestFloorReached, floorsCleared);
        // Traverse only from the node the player actually entered.
        reachable = getReachableIndices(layer, std::vector<int>{chosen});
    }

    const bool playerWon{!party.isAllDead() &&
                         floorsCleared == static_cast<int>(m_layers.size())};

    m_eventBus.emit(RunCompletedEvent{playerWon, floorsCleared});
    m_eventBus.clearRunScope();
    return playerWon;
}

int Dungeon::presentChoices(Party &party,
                            MetaProgress &meta,
                            int layerIndex,
                            const std::vector<int> &reachable)
{
    const auto &layer{m_layers[static_cast<std::size_t>(layerIndex)]};

    std::vector<int> choices{};
    for (int idx : reachable)
        if (idx < static_cast<int>(layer.size()))
            choices.push_back(idx);

    if (choices.empty())
        choices.push_back(0);

    int chosenIndex{choices[0]};

    if (choices.size() == 1)
    {
        std::cout << "Path: "
                  << layer[static_cast<std::size_t>(chosenIndex)].content->description()
                  << "\n";
        layer[static_cast<std::size_t>(chosenIndex)].content->enter(
            party, meta, m_runContext, m_eventBus);
    }
    else
    {
        std::cout << "Choose a path:\n";
        for (std::size_t i{0}; i < choices.size(); ++i)
            std::cout << "  [" << (i + 1) << "] "
                      << layer[static_cast<std::size_t>(choices[i])].content->description()
                      << "\n";

        std::size_t pick{1};
        std::cout << "Path: ";
        std::cin >> pick;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (pick < 1 || pick > choices.size())
            pick = 1;

        chosenIndex = choices[pick - 1];
        layer[static_cast<std::size_t>(chosenIndex)].content->enter(
            party, meta, m_runContext, m_eventBus);
    }

    return party.isAllDead() ? -1 : chosenIndex;
}