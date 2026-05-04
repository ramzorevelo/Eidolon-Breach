/**
 * @file EnemyRegistry.cpp
 * @brief EnemyRegistry implementation.
 */

#include "Entities/EnemyRegistry.h"
#include "Core/DataLoader.h"
#include "Entities/Enemy.h"
#include "Core/Affinity.h"
#include "Entities/IAIStrategy.h"
#include "Entities/Slime.h"
#include "Entities/StoneGolem.h"
#include "Entities/VampireBat.h"
#include <stdexcept>


namespace
{
Affinity parseAffinityStr(const std::string &s)
{
    if (s == "Blaze")
        return Affinity::Blaze;
    if (s == "Frost")
        return Affinity::Frost;
    if (s == "Tempest")
        return Affinity::Tempest;
    if (s == "Terra")
        return Affinity::Terra;
    return Affinity::Aether;
}
} // namespace

void EnemyRegistry::loadFromJson(const std::string &jsonPath)
{
    const nlohmann::json j{DataLoader::loadJson(jsonPath)};
    for (auto it = j.begin(); it != j.end(); ++it)
        m_blueprints[it.key()] = parseBlueprint(it.key(), it.value());
}

std::unique_ptr<Enemy> EnemyRegistry::create(std::string_view id) const
{
    auto it = m_blueprints.find(std::string{id});
    if (it == m_blueprints.end())
        return nullptr;
    return instantiate(it->second);
}

bool EnemyRegistry::contains(std::string_view id) const
{
    return m_blueprints.count(std::string{id}) > 0;
}

std::size_t EnemyRegistry::size() const
{
    return m_blueprints.size();
}

EnemyBlueprint EnemyRegistry::parseBlueprint(const std::string &id,
                                             const nlohmann::json &j)
{
    EnemyBlueprint bp{};
    bp.id = id;
    bp.name = j.at("name").get<std::string>();
    bp.enemyType = j.at("enemyType").get<std::string>();
    bp.maxHp = j.at("maxHp").get<int>();
    bp.maxToughness = j.at("maxToughness").get<int>();
    bp.faction = j.value("faction", "");
    bp.category = j.value("category", "");
    bp.affinity = j.value("affinity", "Aether");
    return bp;
}

std::unique_ptr<Enemy> EnemyRegistry::instantiate(const EnemyBlueprint &bp)
{
    if (bp.enemyType == "slime")
        return std::make_unique<Slime>(bp.name, bp.maxHp, bp.maxToughness);
    if (bp.enemyType == "stone_golem")
        return std::make_unique<StoneGolem>(bp.name, bp.maxHp, bp.maxToughness);
    if (bp.enemyType == "vampire_bat")
        return std::make_unique<VampireBat>(bp.name, bp.maxHp, bp.maxToughness);

    // Fallback: generic enemy with minimal stats. Used for future enemy types
    // defined purely in JSON before a dedicated subclass is written.
    return std::make_unique<Enemy>(
        bp.id, bp.name,
        Stats{bp.maxHp, bp.maxHp, 10, 0, 5},
        parseAffinityStr(bp.affinity),
        bp.maxToughness,
        std::make_unique<BasicAIStrategy>());
}