#pragma once
/**
 * @file Dungeon.h
 * @brief Graph-based dungeon map. Each floor layer holds 1-3 nodes with
 *        directed edges to nodes in the next layer. Paths can converge,
 *        diverge, or run solo. Elite frequency scales with difficulty.
 */

#include "Core/Affinity.h"
#include "Core/EventBus.h"
#include "Items/ItemRegistry.h"
#include "Core/RunContext.h"
#include "Dungeon/MapNode.h"
#include "Dungeon/EncounterTable.h"
#include "Entities/EnemyRegistry.h"
#include "Dungeon/DungeonDefinition.h"
#include <cstdint>
#include <memory>
#include <random>
#include <vector>

class AchievementSystem;
class Party;
class MetaProgress;
class SummonRegistry;


/** One node in the dungeon graph. */
struct DungeonGraphNode
{
    std::unique_ptr<MapNode> content;
    std::vector<int> nextIndices{}; ///< Indices into the next layer's nodes.
};

class Dungeon
{
  public:
    Dungeon();
    ~Dungeon();
    /**
     * @brief Generate a graph-based map from a reproducible seed.
     * @param seed       Unsigned 32-bit seed — never pass a signed int.
     * @param numLayers  Total floor count including boss floor.
     * @param difficulty Controls elite spawn weight and reward scaling.
     */
    void generate(std::uint32_t seed,
                  const DungeonDefinition &dungeonDef,
                  SummonRegistry *summonRegistry = nullptr,
                  RunMode runMode = RunMode::Classic);

    /**
     * @brief Run the dungeon. Returns true if the player clears the boss.
     */
    bool run(Party &party, MetaProgress &meta,
             IRenderer &renderer, IInputHandler &input);

    [[nodiscard]] const DungeonDefinition &getCurrentDungeon() const
    {
        return m_currentDungeon;
    }

  private:
    void assignFloorAffinities(std::uint32_t seed, int numLayers);
    void buildGraph(std::uint32_t seed, int numLayers, DungeonDifficulty difficulty);
    /**
     * @brief Build a single-path (no branches) dungeon from an ordered node list.
     *        Called by generate() when dungeonDef.fixedLayout is non-empty.
     */
    void buildFixedGraph(std::uint32_t seed,
                         const DungeonDefinition &def);

    [[nodiscard]] std::unique_ptr<MapNode> makeNode(int layer,
                                                    int numLayers,
                                                    DungeonDifficulty difficulty,
                                                    std::mt19937 &rng,
                                                    bool noElite = false,
                                                    bool noRest = false,
                                                    bool noTreasure = false,
                                                    bool noBattle = false,
                                                    bool noShop = false) const;

    void connectLayers(int layerIndex, std::mt19937 &rng);

    /// Returns the chosen node index, or -1 if the party died inside the node.
    int presentChoices(Party &party, MetaProgress &meta,
                       int layerIndex, const std::vector<int> &reachable,
                       IRenderer &renderer, IInputHandler &input);

    [[nodiscard]] std::vector<int> getReachableIndices(
        int layerIndex,
        const std::vector<int> &currentIndices) const;

    RunContext m_runContext{};
    EventBus m_eventBus{};
    std::vector<std::vector<DungeonGraphNode>> m_layers{};
    std::vector<Affinity> m_floorAffinities{};
    DungeonDifficulty m_difficulty{DungeonDifficulty::Normal};
    std::unique_ptr<AchievementSystem> m_achievements{};
    SummonRegistry *m_summonRegistry{nullptr}; ///< Non-owning; owned by main().
    EnemyRegistry m_enemyRegistry{};
    EncounterTable m_encounterTable{};
    ItemRegistry m_itemRegistry{};
    DungeonDefinition m_currentDungeon{};
};