/**
 * @file AbilityRegistry.cpp
 * @brief AbilityRegistry implementation.
 */

#include "Characters/AbilityRegistry.h"

void AbilityRegistry::registerAbility(const std::string &id, Factory factory)
{
    m_factories[id] = std::move(factory);
}

std::unique_ptr<IAction> AbilityRegistry::create(std::string_view id) const
{
    auto it = m_factories.find(std::string{id});
    if (it == m_factories.end())
        return nullptr;
    return it->second();
}

bool AbilityRegistry::contains(std::string_view id) const
{
    return m_factories.count(std::string{id}) > 0;
}