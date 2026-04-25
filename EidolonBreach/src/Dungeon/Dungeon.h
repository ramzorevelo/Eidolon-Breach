#pragma once
/**
 * @file Dungeon.h
 * @brief Graph-based dungeon map. Each floor layer holds 1-3 nodes with
 *        directed edges to nodes in the next layer. Paths can converge,
 *        diverge, or run solo. Elite frequency scales with difficulty.
 */

#include "Core/Affinity.h"
#include "Core/EventBus.h"
#include "Core/RunContext.h"
#include "Dungeon/MapNode.h"
#include <cstdint>
#include <memory>
#include <random>
#include <vector>

class AchievementSystem;
class Party;
class MetaProgress;


/** One node in the dungeon graph. */
struct DungeonGraphNode
{
    std::unique_ptr<MapNode> content;
    std::vector<int> nextIndices{}; ///< Indices into the next layer's nodes.
};

/** Difficulty preset controlling elite frequency and gold rewards. */
enum class DungeonDifficulty
{
    Normal,
    Hard,
    Nightmare
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
                  int numLayers,
                  DungeonDifficulty difficulty = DungeonDifficulty::Normal);

    /**
     * @brief Run the dungeon. Returns true if the player clears the boss.
     */
    bool run(Party &party, MetaProgress &meta);

  private:
    void assignFloorAffinities(std::uint32_t seed, int numLayers);
    void buildGraph(std::uint32_t seed, int numLayers, DungeonDifficulty difficulty);

    [[nodiscard]] std::unique_ptr<MapNode> makeNode(int layer,
                                                    int numLayers,
                                                    DungeonDifficulty difficulty,
                                                    std::mt19937 &rng,
                                                    bool noElite = false,
                                                    bool noRest = false,
                                                    bool noTreasure = false) const;

    void connectLayers(int layerIndex, std::mt19937 &rng);

    /// Returns the chosen node index, or -1 if the party died inside the node.
    int presentChoices(Party &party,
                       MetaProgress &meta,
                       int layerIndex,
                       const std::vector<int> &reachable);

    [[nodiscard]] std::vector<int> getReachableIndices(
        int layerIndex,
        const std::vector<int> &currentIndices) const;

    RunContext m_runContext{};
    EventBus m_eventBus{};
    std::vector<std::vector<DungeonGraphNode>> m_layers{};
    std::vector<Affinity> m_floorAffinities{};
    DungeonDifficulty m_difficulty{DungeonDifficulty::Normal};
    std::unique_ptr<AchievementSystem> m_achievements{};
};