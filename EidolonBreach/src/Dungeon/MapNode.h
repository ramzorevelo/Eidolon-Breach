#pragma once
/**
 * @file MapNode.h
 * @brief Abstract base for all dungeon nodes.
 *        MetaProgress is injected by reference — never accessed as a singleton.
 */

#include <string>

class Party;
class MetaProgress;
class RunContext;
class EventBus;

class MapNode
{
  public:
    virtual ~MapNode() = default;

    /**
     * @brief Execute the node's logic for the given party.
     * @param party     The player party entering the node.
     * @param meta      Cross-run persistent state.
     * @param runCtx    Per-run signal and vote tracking state.
     * @param eventBus  Event bus for cross-system notifications.
     */
    virtual void enter(Party &party,
                       MetaProgress &meta,
                       RunContext &runCtx,
                       EventBus &eventBus) = 0;

    /** @return One-line description shown on the map screen. */
    [[nodiscard]] virtual std::string description() const = 0;
};