#pragma once
/**
 * @file SummonRegistry.h
 * @brief Stores all SummonDefinitions keyed by string ID.
 *        No global state. Constructed in main() and injected into Dungeon.
 */

#include "Summons/SummonDefinition.h"
#include <string_view>
#include <unordered_map>

class SummonRegistry
{
  public:
    /**
     * @brief Move a definition into the registry.
     *        Overwrites any existing entry with the same id.
     */
    void registerDefinition(SummonDefinition def);

    /**
     * @brief Look up a definition by string ID.
     * @return Pointer to the stored definition, or nullptr if not found.
     */
    [[nodiscard]] const SummonDefinition *find(std::string_view id) const;

  private:
    std::unordered_map<std::string, SummonDefinition> m_defs{};
};