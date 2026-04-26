#pragma once
/**
 * @file EncounterTable.h
 * @brief Loads encounter group definitions from JSON and produces enemy-party
 *        factory functions consumable by BattleNode, EliteNode, and BossNode.
 *        An encounter group is a named list of enemy IDs drawn from EnemyRegistry.
 */

#include <functional>
#include <random>
#include <string>
#include <vector>
#include "nlohmann/json.hpp"

class Party;
class EnemyRegistry;

struct EncounterGroup
{
    std::string id{};
    std::vector<std::string> enemyIds{};
};

class EncounterTable
{
  public:
    enum class Tier
    {
        Standard,
        Elite,
        Boss
    };

    /**
     * @brief Load encounter definitions. Must be called before getFactory().
     * @param jsonPath      Path to encounters.json.
     * @param enemyRegistry Used to validate that all enemy IDs are registered.
     */
    void loadFromJson(const std::string &jsonPath,
                      const EnemyRegistry &enemyRegistry);

    /**
     * @brief Return a factory function that populates an enemy Party from a
     *        randomly selected encounter group in the given tier.
     * @param tier Difficulty tier for this node type.
     * @param rng  RNG owned by Dungeon (passed by reference for reproducibility).
     * @return std::function<void(Party&)> compatible with BattleNode's constructor.
     */
    [[nodiscard]] std::function<void(Party &)>
    getFactory(Tier tier, std::mt19937 &rng) const;

    [[nodiscard]] bool hasTier(Tier tier) const;

  private:
    std::vector<EncounterGroup> m_standard{};
    std::vector<EncounterGroup> m_elite{};
    std::vector<EncounterGroup> m_boss{};

    const EnemyRegistry *m_registry{nullptr};

    static std::vector<EncounterGroup>
    parseGroups(const nlohmann::json &arr, const EnemyRegistry &reg);
};