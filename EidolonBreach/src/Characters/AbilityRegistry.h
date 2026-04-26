#pragma once
/**
 * @file AbilityRegistry.h
 * @brief Maps string ability IDs to factory functions that produce IAction instances.
 *        Abilities are C++ logic and cannot be fully expressed as data, so the
 *        registry bridges JSON ability ID strings to the correct concrete IAction.
 *
 *        Register all abilities before CharacterRegistry::create() is called.
 *        No global state — constructed and injected.
 */

#include "Actions/IAction.h"
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

class AbilityRegistry
{
  public:
    using Factory = std::function<std::unique_ptr<IAction>()>;

    /** @brief Register a factory for the given ability ID. Overwrites if already set. */
    void registerAbility(const std::string &id, Factory factory);

    /**
     * @brief Create an IAction instance for the given ability ID.
     * @return nullptr if the ID is not registered.
     */
    [[nodiscard]] std::unique_ptr<IAction> create(std::string_view id) const;

    [[nodiscard]] bool contains(std::string_view id) const;

  private:
    std::unordered_map<std::string, Factory> m_factories{};
};