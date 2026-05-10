#pragma once
/**
 * @file ShopNode.h
 * @brief Map node that sells consumables from ItemRegistry for gold.
 *        Console UI: displays stock, player picks item by number, gold deducted.
 */
#include "Dungeon/MapNode.h"
#include "Items/ItemRegistry.h"
#include <cstdint>

class IRenderer;
class IInputHandler;

class ShopNode : public MapNode
{
  public:
    /**
     * @param registry    Shared item registry. Must outlive this node.
     * @param stockIds    IDs of items offered. Caller controls stock composition.
     * @param rngSeed     Seed used when randomly rolling stock.
     */
    explicit ShopNode(const ItemRegistry &registry,
                      std::vector<std::string> stockIds,
                      std::uint32_t rngSeed = 0u);

    void enter(Party &party, MetaProgress &meta,
               RunContext &runCtx, EventBus &eventBus,
               IRenderer &renderer, IInputHandler &input) override;

    [[nodiscard]] std::string description() const override;
    [[nodiscard]] NodeType nodeType() const override
    {
        return NodeType::Shop;
    }
  private:
    const ItemRegistry &m_registry;
    std::vector<std::string> m_stockIds;
};