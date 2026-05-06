/**
 * @file ShopNode.cpp
 * @brief ShopNode implementation.
 */
#include "Dungeon/ShopNode.h"
#include "Entities/Party.h"
#include "Items/Inventory.h"
#include <limits>
#include "UI/IRenderer.h"
#include "UI/IInputHandler.h"
#include <optional>

ShopNode::ShopNode(const ItemRegistry &registry,
                   std::vector<std::string> stockIds,
                   std::uint32_t /*rngSeed*/)
    : m_registry{registry}, m_stockIds{std::move(stockIds)}
{
}

void ShopNode::enter(Party &party, MetaProgress & /*meta*/,
                     RunContext & /*runCtx*/, EventBus & /*eventBus*/,
                     IRenderer &renderer, IInputHandler &input)
{
    std::vector<Item> stock{};
    for (const std::string &id : m_stockIds)
    {
        auto item = m_registry.create(id);
        if (item.has_value())
            stock.push_back(std::move(*item));
    }

    std::vector<std::string> options{};
    options.push_back("Leave");
    for (const auto &item : stock)
        options.push_back(item.name + "  (" + std::to_string(item.goldValue) + " gold)");

    const std::string title = "SHOP  —  Gold: " + std::to_string(party.getInventory().gold);

    if (stock.empty())
    {
        renderer.renderMessage("The shop shelves are empty.");
        return;
    }

    input.setMenuContext(title, options);
    renderer.renderSelectionMenu(title, options);
    const std::size_t choice = input.getMenuChoice(options.size());

    if (choice == 0)
    {
        renderer.renderMessage("You leave without buying anything.");
        return;
    }

    const Item &selected = stock[choice - 1];

    if (party.getInventory().gold < selected.goldValue)
    {
        renderer.renderMessage("Not enough gold.");
        return;
    }

    party.getInventory().gold -= selected.goldValue;
    party.getInventory().addItem(selected, 1);
    renderer.renderMessage("Purchased " + selected.name + ". Gold remaining: " + std::to_string(party.getInventory().gold));
}

std::string ShopNode::description() const
{
    return "[Shop] Buy consumables with gold.";
}