/**
 * @file EventBus.cpp
 * @brief EventBus scope-clearing implementation.
 */

#include "Core/EventBus.h"
#include <algorithm>

void EventBus::clearBattleScope()
{
    clearScope(EventScope::Battle);
}

void EventBus::clearRunScope()
{
    clearScope(EventScope::Run);
}

void EventBus::clearScope(EventScope scope)
{
    for (auto &[key, entries] : m_handlers)
    {
        entries.erase(
            std::remove_if(entries.begin(), entries.end(),
                           [scope](const HandlerEntry &e)
                           { return e.scope == scope; }),
            entries.end());
    }
}