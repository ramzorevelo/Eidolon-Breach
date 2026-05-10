#pragma once
/**
 * @file EnemyRegistry.h
 * @brief Loads enemy blueprints from JSON and creates Enemy instances on demand.
 *        Enemy subclasses (Slime, StoneGolem, VampireBat) are selected via
 *        the "enemyType" field so break effects remain in C++ without downcasting.
 */

#include "Core/Drop.h"
#include "Items/Item.h" 
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include "nlohmann/json.hpp"

class Enemy;

struct EnemyBlueprint
{
    std::string id{};
    std::string name{};
    std::string enemyType{};
    int maxHp{};
    int maxToughness{};
    std::string faction{};
    std::string category{};
    std::string affinity{"Aether"};
    std::vector<Drop> drops{};
};

class EnemyRegistry
{
  public:
    void loadFromJson(const std::string &jsonPath);

    /**
     * @brief Instantiate a fresh Enemy from the blueprint with the given ID.
     * @return nullptr if the ID is not registered.
     */
    [[nodiscard]] std::unique_ptr<Enemy> create(std::string_view id) const;

    [[nodiscard]] bool contains(std::string_view id) const;
    [[nodiscard]] std::size_t size() const;

  private:
    std::unordered_map<std::string, EnemyBlueprint> m_blueprints{};

    static EnemyBlueprint parseBlueprint(const std::string &id,
                                         const nlohmann::json &j);
    static std::unique_ptr<Enemy> instantiate(const EnemyBlueprint &bp);
    
};