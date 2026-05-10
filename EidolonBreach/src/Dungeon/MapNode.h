#pragma once
/**
 * @file MapNode.h
 * @brief Abstract base for all dungeon nodes.
 *        MetaProgress is injected by reference.
 */

#include <string>

class Party;
class MetaProgress;
class RunContext;
class EventBus;
class IRenderer;
class IInputHandler;


class MapNode
{
  public:
    virtual ~MapNode() = default;

    virtual void enter(Party &party,
                       MetaProgress &meta,
                       RunContext &runCtx,
                       EventBus &eventBus,
                       IRenderer &renderer,
                       IInputHandler &input) = 0;

    /** @return One-line description shown on the map screen. */
    [[nodiscard]] virtual std::string description() const = 0;
    enum class NodeType
    {
        Battle,
        Elite,
        Boss,
        Rest,
        Treasure,
        Shop,
        Event
    };

    /**
     * @brief Returns the node's type for dungeon generation constraint checks.
     */
    [[nodiscard]] virtual NodeType nodeType() const = 0;
};