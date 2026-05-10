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
    if (j.contains("drops"))
    {
        for (const auto &dropJson : j.at("drops"))
        {
            Drop d{};
            const std::string dtype{dropJson.at("type").get<std::string>()};
            if (dtype == "gold")
                d.type = Drop::Type::Gold;
            else if (dtype == "guaranteed_item")
                d.type = Drop::Type::GuaranteedItem;
            else
                d.type = Drop::Type::Item;
            d.goldAmount = dropJson.value("goldAmount", 0);
            d.itemId = dropJson.value("itemId", "");
            d.dropChance = dropJson.value("dropChance", 1.0f);
            bp.drops.push_back(d);
        }
    }
    return bp;
}

std::unique_ptr<Enemy> EnemyRegistry::instantiate(const EnemyBlueprint &bp)
{
    std::unique_ptr<Enemy> enemy{};

    if (bp.enemyType == "slime")
        enemy = std::make_unique<Slime>(bp.name, bp.maxHp, bp.maxToughness);
    else if (bp.enemyType == "stone_golem")
        enemy = std::make_unique<StoneGolem>(bp.name, bp.maxHp, bp.maxToughness);
    else if (bp.enemyType == "vampire_bat")
        enemy = std::make_unique<VampireBat>(bp.name, bp.maxHp, bp.maxToughness);
    else
        enemy = std::make_unique<Enemy>(
            bp.id, bp.name,
            Stats{bp.maxHp, bp.maxHp, 10, 0, 5},
            parseAffinityStr(bp.affinity),
            bp.maxToughness,
            std::make_unique<BasicAIStrategy>());

    for (const Drop &drop : bp.drops)
        enemy->addDrop(drop);

    return enemy;
}