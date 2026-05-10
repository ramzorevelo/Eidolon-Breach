/**
 * @file Dungeon.cpp
 * @brief Graph-based dungeon generation and run-loop.
 */

#include "Dungeon/Dungeon.h"
#include "Core/AchievementSystem.h"
#include "Core/FieldDiscovery.h"
#include "Core/BattleEvents.h"
#include "Core/CombatConstants.h"
#include "Core/MetaProgress.h"
#include "Dungeon/BattleNode.h"
#include "Dungeon/EncounterTable.h"
#include "Entities/EnemyRegistry.h"
#include "UI/IRenderer.h"
#include "UI/IInputHandler.h"
#include "Dungeon/ShopNode.h"
#include "Items/ItemRegistry.h"
#include "Dungeon/BossNode.h"
#include "Entities/PlayableCharacter.h"
#include "Dungeon/EliteNode.h"
#include "Dungeon/EventNode.h"
#include "Dungeon/RestNode.h"
#include "Dungeon/TreasureNode.h"
#include "Entities/Party.h"
#include "Dungeon/DungeonDefinition.h"
#include "Dungeon/DungeonTable.h"
#include <cmath>
#include "Summons/SummonRegistry.h"
#include <algorithm>
#include <limits>
#include <random>

namespace
{
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
    return base + static_cast<int>(depthRatio * 3.0f);
}
} // namespace
Dungeon::Dungeon() = default;
Dungeon::~Dungeon() = default;

void Dungeon::generate(std::uint32_t seed,
                       const DungeonDefinition &dungeonDef,
                       SummonRegistry *summonRegistry,
                       RunMode runMode)
{
    m_currentDungeon = dungeonDef;
    m_summonRegistry = summonRegistry;
    m_difficulty = dungeonDef.difficulty;
    m_runContext.reset();
    m_runContext.runMode = runMode;

    constexpr std::uint32_t kDraftSeedOffset{0xDEADBEEFu};
    const std::uint32_t effectiveSeed{
        runMode == RunMode::EidolonLabyrinth ? seed ^ kDraftSeedOffset : seed};

    m_achievements = std::make_unique<AchievementSystem>(m_eventBus);
    m_layers.clear();
    m_enemyRegistry.loadFromJson("data/enemies.json");
    m_itemRegistry.loadFromJson("data/items.json");
    m_encounterTable.loadFromJson("data/encounters.json", m_enemyRegistry);
    const int floors{dungeonDef.fixedLayout.empty()
                         ? dungeonDef.numFloors
                         : static_cast<int>(dungeonDef.fixedLayout.size())};
    assignFloorAffinities(effectiveSeed, floors);

    if (!dungeonDef.fixedLayout.empty())
        buildFixedGraph(effectiveSeed, dungeonDef.fixedLayout, dungeonDef.difficulty);
    else
        buildGraph(effectiveSeed, floors, dungeonDef.difficulty);
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
                                           bool noTreasure,
                                           bool noBattle,
                                           bool noShop) const
{
    const Affinity floorAffinity{
        m_floorAffinities[static_cast<std::size_t>(layer)]};
    const int gold{20 + layer * 5};

    const int eW{noElite ? 0 : eliteWeight(layer, numLayers, difficulty)};
    const int battleW{noBattle ? 0 : 3};
    int restW{noRest ? 0 : 4};
    const int treasureW{noTreasure ? 0 : 3};
    const int shopW{noShop || layer < 2 ? 0 : 2}; // No shops on first two floors.

    if (eW == 0 && battleW == 0 && treasureW == 0 && shopW == 0)
        restW = 4;

    const int totalW{eW + battleW + restW + treasureW + shopW};

    std::uniform_int_distribution<int> dist{0, totalW - 1};
    const int roll{dist(rng)};

    if (roll < eW)
        return std::make_unique<EliteNode>(
            m_encounterTable.getFactory(EncounterTable::Tier::Elite, rng),
            floorAffinity, m_currentDungeon.enemyLevel, m_summonRegistry);
    if (roll < eW + battleW)
        return std::make_unique<BattleNode>(
            m_encounterTable.getFactory(EncounterTable::Tier::Standard, rng),
            floorAffinity, m_currentDungeon.enemyLevel, m_summonRegistry);
    if (roll < eW + battleW + restW)
        return std::make_unique<RestNode>();
    if (roll < eW + battleW + restW + shopW)
    {
        // Stock: one of each consumable, rotated by floor.
        std::vector<std::string> stock{"heal_potion", "purification_vial"};
        if (layer % 2 == 0)
            stock.push_back("mega_potion");
        return std::make_unique<ShopNode>(m_itemRegistry, std::move(stock), rng());
    }
    return std::make_unique<TreasureNode>(gold, rng());
}

void Dungeon::buildGraph(std::uint32_t seed,
                         int numLayers,
                         DungeonDifficulty difficulty)
{
    std::mt19937 rng{seed};

    bool prevLayerHadElite{false};
    bool hasSeenCombat{false};
    bool prevLayerHadShop{false};
    bool prevLayerHadRest{false};
    bool prevLayerHadTreasure{false};
    int consecutiveBattleFloors{0};

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
                                      m_encounterTable.getFactory(EncounterTable::Tier::Boss, rng),
                                      floorAffinity, m_currentDungeon.enemyLevel, m_summonRegistry),
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
                                      m_encounterTable.getFactory(EncounterTable::Tier::Elite, rng),
                                      floorAffinity, m_currentDungeon.enemyLevel, m_summonRegistry),
                                  {}});
            prevLayerHadElite = true;
            prevLayerHadRest = false;
            prevLayerHadTreasure = false;
        }
        else
        {
            // Early floors always offer branching. Near-boss floors may collapse to 1 node.
            const int minWidth{(layer < numLayers - 5) ? 2 : 1};
            std::uniform_int_distribution<int> localWidthDist{minWidth, 3};
            const int width{localWidthDist(rng)};
            bool thisLayerHadElite{false};
            bool thisLayerHadRest{false};
            bool thisLayerHadTreasure{false};
            const bool isPreEliteGate{layer == numLayers - 4};
            const bool noBattle{consecutiveBattleFloors >= 2};
            for (int i{0}; i < width; ++i)
            {
                const bool noRest{prevLayerHadRest || !hasSeenCombat || layer == 0};
                auto node{makeNode(layer, numLayers, difficulty, rng,
                                   layer < 2 || prevLayerHadElite || isPreEliteGate,
                                   noRest,
                                   prevLayerHadTreasure,
                                   noBattle,
                                   prevLayerHadShop)};
                if (node->nodeType() == MapNode::NodeType::Elite)
                    thisLayerHadElite = true;
                if (node->nodeType() == MapNode::NodeType::Rest)
                    thisLayerHadRest = true;
                if (node->nodeType() == MapNode::NodeType::Treasure)
                    thisLayerHadTreasure = true;
                layerNodes.push_back({std::move(node), {}});
            }
            prevLayerHadElite = thisLayerHadElite;
            prevLayerHadRest = thisLayerHadRest;
            prevLayerHadTreasure = thisLayerHadTreasure;

                        // Track whether this layer contains a ShopNode.
            bool thisLayerHadShop{false};
            for (const auto &node : layerNodes)
            {
                if (node.content->nodeType() == MapNode::NodeType::Shop)
                {
                    thisLayerHadShop = true;
                    break;
                }
            }
            prevLayerHadShop = thisLayerHadShop;

            // Mark that the party has seen combat once any battle node appears.
            for (const auto &n : layerNodes)
            {
                if (n.content->nodeType() == MapNode::NodeType::Battle) 
                {
                    hasSeenCombat = true;
                    break;
                }
            }

            const bool allBattleThisLayer{
                !thisLayerHadElite && !thisLayerHadRest && !thisLayerHadTreasure};
            consecutiveBattleFloors = allBattleThisLayer ? consecutiveBattleFloors + 1 : 0;
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
void Dungeon::buildFixedGraph(std::uint32_t seed,
                              const std::vector<std::string> &layout,
                              DungeonDifficulty difficulty)
{
    (void)difficulty; // reserved for future per-node difficulty scaling
    std::mt19937 rng{seed};
    m_layers.clear();

    for (std::size_t i{0}; i < layout.size(); ++i)
    {
        const Affinity floorAffinity{
            i < m_floorAffinities.size()
                ? m_floorAffinities[i]
                : Affinity::Aether};
        const std::string &nodeType{layout[i]};

        std::unique_ptr<MapNode> node{};
        if (nodeType == "battle")
        {
            node = std::make_unique<BattleNode>(
                m_encounterTable.getFactory(EncounterTable::Tier::Standard, rng),
                floorAffinity, m_currentDungeon.enemyLevel, m_summonRegistry);
        }
        else if (nodeType == "elite")
        {
            node = std::make_unique<EliteNode>(
                m_encounterTable.getFactory(EncounterTable::Tier::Elite, rng),
                floorAffinity, m_currentDungeon.enemyLevel, m_summonRegistry);
        }
        else if (nodeType == "boss")
        {
            node = std::make_unique<BossNode>(
                m_encounterTable.getFactory(EncounterTable::Tier::Boss, rng),
                floorAffinity, m_currentDungeon.enemyLevel, m_summonRegistry);
        }
        else if (nodeType == "rest")
        {
            node = std::make_unique<RestNode>();
        }
        else if (nodeType == "treasure")
        {
            node = std::make_unique<TreasureNode>(
                20 + static_cast<int>(i) * 5, rng());
        }
        else if (nodeType == "shop")
        {
            std::vector<std::string> stock{"heal_potion", "purification_vial"};
            if (static_cast<int>(i) % 2 == 0)
                stock.push_back("mega_potion");
            node = std::make_unique<ShopNode>(m_itemRegistry, std::move(stock), rng());
        }
        else
        {
            // "event" and any unknown type fall back to EventNode
            node = std::make_unique<EventNode>();
        }

        std::vector<DungeonGraphNode> layer{};
        layer.push_back({std::move(node), {}});
        m_layers.push_back(std::move(layer));
    }

    // Connect each layer to the next (linear, no branching).
    for (std::size_t i{0}; i + 1 < m_layers.size(); ++i)
        m_layers[i][0].nextIndices.push_back(0);
}
bool Dungeon::run(Party &party, MetaProgress &meta,
                  IRenderer &renderer, IInputHandler &input)
{
    int floorsCleared{0};

    // Player may start from any node on floor 0.
    std::vector<int> reachable{};
    // Track stance crystallizations for Mastery Event logging.
    m_eventBus.subscribe<StanceCrystallizedEvent>(
        [&meta](const StanceCrystallizedEvent &e)
        {
            if (!e.character || e.stanceId.empty())
                return;
            const std::string charId{e.character->getId()};
            auto &log{meta.masteryEventLog[charId]};
            // Only record each stance once across all runs in the log.
            for (const std::string &recorded : log)
                if (recorded == e.stanceId)
                    return;
            log.push_back(e.stanceId);
        },
        EventScope::Run);
    for (int i{0}; i < static_cast<int>(m_layers[0].size()); ++i)
        reachable.push_back(i);

   for (int layer{0}; layer < static_cast<int>(m_layers.size()); ++layer)
    {
        if (party.isAllDead())
            break;

        renderer.renderMessage("--- Floor " + std::to_string(layer + 1) + "/" + std::to_string(m_layers.size()) + "  Affinity: " + affinityToString(m_floorAffinities[static_cast<std::size_t>(layer)]) + " ---");

        const int chosen = presentChoices(party, meta, layer, reachable,
                                          renderer, input);
        if (chosen == -1)
            break;

        ++floorsCleared;
        meta.highestFloorReached = std::max(meta.highestFloorReached, floorsCleared);

        const std::size_t discoveriesBefore = m_runContext.activeDiscoveries.size();
        m_runContext.checkAndActivateDiscoveries();
        if (m_runContext.activeDiscoveries.size() > discoveriesBefore)
            renderer.renderMessage(">> FIELD DISCOVERY ACTIVATED! <<");

        reachable = getReachableIndices(layer, std::vector<int>{chosen});
    }

    const bool playerWon = !party.isAllDead() && floorsCleared == static_cast<int>(m_layers.size());
    if (playerWon)
    {
        const bool isFirstClear{meta.clearedDungeonIds.count(m_currentDungeon.id) == 0};
        if (isFirstClear)
            meta.clearedDungeonIds.insert(m_currentDungeon.id);

        const float gap{static_cast<float>(
            m_currentDungeon.enemyLevel - meta.playerLevel)};
        const float scale{std::clamp(
            1.0f + gap * CombatConstants::kPlayerXpLevelScale, 0.1f, 2.0f)};
        int xpAwarded{static_cast<int>(
            static_cast<float>(CombatConstants::kPlayerXpDungeonBase) * scale)};
        if (isFirstClear)
            xpAwarded += CombatConstants::kPlayerXpFirstClearBonus;

        const int newPlayerLevel{meta.gainPlayerXp(xpAwarded)};
        renderer.renderMessage("Player XP gained: " + std::to_string(xpAwarded)
            + "  (Player Level: " + std::to_string(newPlayerLevel) + ")");
    }

    if (m_runContext.runMode == RunMode::Classic)
        meta.gainRunSignals(m_runContext, party);

    m_eventBus.emit(RunCompletedEvent{playerWon, floorsCleared});
    m_eventBus.clearRunScope();
    return playerWon;
}

int Dungeon::presentChoices(Party &party, MetaProgress &meta,
                            int layerIndex,
                            const std::vector<int> &reachable,
                            IRenderer &renderer, IInputHandler &input)
{
    const auto &layer = m_layers[static_cast<std::size_t>(layerIndex)];

    std::vector<int> choices{};
    for (int idx : reachable)
        if (idx < static_cast<int>(layer.size()))
            choices.push_back(idx);
    if (choices.empty())
        choices.push_back(0);

    int chosenIndex = choices[0];

    if (choices.size() == 1)
    {
        renderer.renderMessage("Path: " + layer[static_cast<std::size_t>(chosenIndex)].content->description());
        renderer.presentPause(300);
        layer[static_cast<std::size_t>(chosenIndex)].content->enter(
            party, meta, m_runContext, m_eventBus, renderer, input);
    }
    else
    {
        std::vector<std::string> options{};
        for (int idx : choices)
            options.push_back(
                layer[static_cast<std::size_t>(idx)].content->description());

        input.setMenuContext("CHOOSE PATH", options);
        renderer.renderSelectionMenu("CHOOSE PATH", options);
        const std::size_t pick = input.getMenuChoice(options.size());

        chosenIndex = choices[pick];
        layer[static_cast<std::size_t>(chosenIndex)].content->enter(
            party, meta, m_runContext, m_eventBus, renderer, input);
    }

    return party.isAllDead() ? -1 : chosenIndex;
}