/**
 * @file BackpackScreen.cpp
 * @brief BackpackScreen implementation.
 */
#include "UI/BackpackScreen.h"
#include "Items/Inventory.h"
#include "Items/Item.h"
#include "UI/IInputHandler.h"
#include "UI/SDL3InputHandler.h"
#include "UI/SDL3Renderer.h"
#include <optional>
#include <string>
#include <vector>

namespace
{
std::string rarityLabel(ItemRarity r)
{
    switch (r)
    {
    case ItemRarity::Common:
        return "[Common]";
    case ItemRarity::Rare:
        return "[Rare]";
    case ItemRarity::Epic:
        return "[Epic]";
    }
    return "";
}

std::string slotLabel(std::optional<EquipSlot> slot)
{
    if (!slot)
        return "";
    switch (*slot)
    {
    case EquipSlot::Weapon:
        return "Weapon";
    case EquipSlot::Armor:
        return "Armor";
    case EquipSlot::Accessory:
        return "Accessory";
    }
    return "";
}
} // namespace

void BackpackScreen::showTab(Tab tab,
                             const Inventory &inventory,
                             SDL3Renderer &renderer,
                             SDL3InputHandler &input)
{
    std::string title{};
    std::vector<std::string> opts{};

    switch (tab)
    {
    case Tab::Consumables:
    {
        title = "BACKPACK -- Consumables";
        const auto &consumables{inventory.getConsumables()};
        if (consumables.empty())
            opts.push_back("(empty)");
        else
            for (const auto &[item, count] : consumables)
                opts.push_back(item.name + "  x" + std::to_string(count));
        break;
    }
    case Tab::Equipment:
    {
        title = "BACKPACK -- Equipment";
        const auto &equip{inventory.getEquipment()};
        if (equip.empty())
            opts.push_back("(empty)");
        else
            for (const auto &item : equip)
                opts.push_back(slotLabel(item.equipSlot) + "  " +
                               rarityLabel(item.rarity) + "  " + item.name);
        break;
    }
    case Tab::KeyItems:
        title = "BACKPACK -- Key Items";
        opts.push_back("(empty)");
        break;
    }

    opts.push_back("<< Back");
    input.setMenuContext(title, opts);
    renderer.renderSelectionMenu(title, opts);
    input.getMenuChoice(opts.size());
}

void BackpackScreen::run(const Inventory &inventory,
                         SDL3Renderer &renderer,
                         SDL3InputHandler &input)
{
    while (true)
    {
        const std::vector<std::string> tabs{
            "Consumables",
            "Equipment",
            "Key Items",
            "<< Back",
        };
        input.setMenuContext("BACKPACK", tabs);
        renderer.renderSelectionMenu("BACKPACK", tabs);
        const std::size_t pick{input.getMenuChoice(tabs.size())};

        if (pick == IInputHandler::kCancelChoice || pick == 3)
            return;
        if (pick == 0)
            showTab(Tab::Consumables, inventory, renderer, input);
        else if (pick == 1)
            showTab(Tab::Equipment, inventory, renderer, input);
        else if (pick == 2)
            showTab(Tab::KeyItems, inventory, renderer, input);
    }
}