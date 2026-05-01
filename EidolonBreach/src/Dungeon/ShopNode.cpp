/**
 * @file ShopNode.cpp
 * @brief ShopNode implementation.
 */
#include "Dungeon/ShopNode.h"
#include "Entities/Party.h"
#include "Items/Inventory.h"
#include <iostream>
#include <limits>
#include <optional>

ShopNode::ShopNode(const ItemRegistry &registry,
                   std::vector<std::string> stockIds,
                   std::uint32_t /*rngSeed*/)
    : m_registry{registry}, m_stockIds{std::move(stockIds)}
{
}

void ShopNode::enter(Party &party,
                     MetaProgress & /*meta*/,
                     RunContext & /*runCtx*/,
                     EventBus & /*eventBus*/)
{
    std::cout << "\n=== SHOP ===\n"
              << "  Gold: " << party.getInventory().gold << '\n';

    std::vector<Item> stock{};
    for (const std::string &id : m_stockIds)
    {
        auto item{m_registry.create(id)};
        if (item.has_value())
            stock.push_back(std::move(*item));
    }

    if (stock.empty())
    {
        std::cout << "  The shop shelves are empty.\n";
        return;
    }

    for (std::size_t i{0}; i < stock.size(); ++i)
    {
        const Item &item{stock[i]};
        std::cout << "  [" << (i + 1) << "] "
                  << item.name
                  << "  (" << item.goldValue << " gold)\n";
    }
    std::cout << "  [0] Leave\n"
              << "Buy: ";

    int choice{0};
    std::cin >> choice;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    if (choice < 1 || choice > static_cast<int>(stock.size()))
    {
        std::cout << "  You leave without buying anything.\n";
        return;
    }

    const Item &selected{stock[static_cast<std::size_t>(choice - 1)]};

    if (party.getInventory().gold < selected.goldValue)
    {
        std::cout << "  Not enough gold.\n";
        return;
    }

    party.getInventory().gold -= selected.goldValue;
    party.getInventory().addItem(selected, 1);
    std::cout << "  Purchased " << selected.name
              << ". Gold remaining: " << party.getInventory().gold << '\n';
}

std::string ShopNode::description() const
{
    return "[Shop] Buy consumables with gold.";
}