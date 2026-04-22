#pragma once
/**
 * @file EventBus.h
 * @brief Synchronous typed event bus with scope-based subscription cleanup.
 *        Owned by Dungeon. Injected into BattleState. No global state.
 */

#include <functional>
#include <typeindex>
#include <unordered_map>
#include <vector>

enum class EventScope
{
    Battle,
    Run
};

class EventBus
{
  public:
    template <typename EventT>
    using Handler = std::function<void(const EventT &)>;

    /**
     * @brief Subscribe a typed handler. Fires in subscription order.
     *        Scope determines which clear call removes it.
     */
    template <typename EventT>
    void subscribe(Handler<EventT> handler, EventScope scope = EventScope::Battle)
    {
        m_handlers[std::type_index(typeid(EventT))].push_back(
            {scope,
             [h = std::move(handler)](const void *e)
             { h(*static_cast<const EventT *>(e)); }});
    }

    /** @brief Emit EventT synchronously to all matching subscribers. */
    template <typename EventT>
    void emit(const EventT &event)
    {
        auto it{m_handlers.find(std::type_index(typeid(EventT)))};
        if (it == m_handlers.end())
            return;
        for (const auto &entry : it->second)
            entry.handler(static_cast<const void *>(&event));
    }

    /** @brief Remove all Battle-scoped subscriptions. Call at battle end. */
    void clearBattleScope();

    /** @brief Remove all Run-scoped subscriptions. Call at run end. */
    void clearRunScope();

  private:
    struct HandlerEntry
    {
        EventScope scope{};
        std::function<void(const void *)> handler{};
    };

    void clearScope(EventScope scope);

    std::unordered_map<std::type_index, std::vector<HandlerEntry>> m_handlers{};
};