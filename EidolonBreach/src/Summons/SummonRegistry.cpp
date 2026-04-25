/**
 * @file SummonRegistry.cpp
 * @brief SummonRegistry implementation.
 */

#include "Summons/SummonRegistry.h"

void SummonRegistry::registerDefinition(SummonDefinition def)
{
    const std::string key{def.id};
    m_defs[key] = std::move(def);
}

const SummonDefinition *SummonRegistry::find(std::string_view id) const
{
    auto it{m_defs.find(std::string{id})};
    return (it != m_defs.end()) ? &it->second : nullptr;
}