/**
 * @file EncounterTable.cpp
 * @brief EncounterTable implementation.
 */

#include "Dungeon/EncounterTable.h"
#include "Core/DataLoader.h"
#include "Entities/EnemyRegistry.h"
#include "Entities/Enemy.h"
#include "Entities/Party.h"
#include <stdexcept>

void EncounterTable::loadFromJson(const std::string &jsonPath,
                                  const EnemyRegistry &enemyRegistry)
{
    m_registry = &enemyRegistry;
    const nlohmann::json j{DataLoader::loadJson(jsonPath)};

    if (j.contains("standard"))
        m_standard = parseGroups(j.at("standard"), enemyRegistry);
    if (j.contains("elite"))
        m_elite = parseGroups(j.at("elite"), enemyRegistry);
    if (j.contains("boss"))
        m_boss = parseGroups(j.at("boss"), enemyRegistry);
}

std::function<void(Party &)>
EncounterTable::getFactory(Tier tier, std::mt19937 &rng) const
{
    const std::vector<EncounterGroup> *pool{nullptr};
    switch (tier)
    {
    case Tier::Standard:
        pool = &m_standard;
        break;
    case Tier::Elite:
        pool = &m_elite;
        break;
    case Tier::Boss:
        pool = &m_boss;
        break;
    }

    if (!pool || pool->empty())
        throw std::runtime_error{"EncounterTable: no groups for requested tier"};

    std::uniform_int_distribution<std::size_t> dist{0, pool->size() - 1};
    const EncounterGroup &group{(*pool)[dist(rng)]};

    // Capture enemy IDs and a pointer to the registry; both outlive the lambda.
    const EnemyRegistry *reg{m_registry};
    const std::vector<std::string> ids{group.enemyIds};

    return [reg, ids](Party &party)
    {
        for (const std::string &id : ids)
        {
            auto enemy{reg->create(id)};
            if (enemy)
                party.addUnit(std::move(enemy));
        }
    };
}

bool EncounterTable::hasTier(Tier tier) const
{
    switch (tier)
    {
    case Tier::Standard:
        return !m_standard.empty();
    case Tier::Elite:
        return !m_elite.empty();
    case Tier::Boss:
        return !m_boss.empty();
    }
    return false;
}

std::vector<EncounterGroup>
EncounterTable::parseGroups(const nlohmann::json &arr,
                            const EnemyRegistry &reg)
{
    std::vector<EncounterGroup> groups{};
    for (const auto &entry : arr)
    {
        EncounterGroup g{};
        g.id = entry.at("id").get<std::string>();
        for (const auto &eid : entry.at("enemies"))
        {
            const std::string id{eid.get<std::string>()};
            if (!reg.contains(id))
                throw std::runtime_error{"EncounterTable: unknown enemy id '" + id + "'"};
            g.enemyIds.push_back(id);
        }
        groups.push_back(std::move(g));
    }
    return groups;
}