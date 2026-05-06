/**
 * @file RestNode.cpp
 * @brief RestNode implementation.
 */
#include "Dungeon/RestNode.h"
#include "Actions/SlotState.h"
#include "Core/CombatConstants.h"
#include "Core/RunContext.h"
#include "Entities/Party.h"
#include "Entities/PlayableCharacter.h"
#include "Entities/Unit.h"
#include "Items/Item.h"
#include "UI/IInputHandler.h"
#include "UI/IRenderer.h"

void RestNode::enter(Party &party, MetaProgress &meta,
                     RunContext &runCtx, EventBus &eventBus,
                     IRenderer &renderer, IInputHandler &input)
{
    const bool isDraft = (runCtx.runMode == RunMode::EidolonLabyrinth);

    std::vector<std::string> options{};
    options.push_back("Heal — restore partial HP to all allies");
    options.push_back("Purge — reduce all Exposure by " + std::to_string(CombatConstants::kPurgeExposureReduction));
    options.push_back("Equip — equip items from party inventory");
    if (isDraft)
        options.push_back("Attune — re-equip slot skills");
    options.push_back("Continue");

    input.setMenuContext("REST SITE", options);
    renderer.renderSelectionMenu("REST SITE", options);
    const std::size_t choice = input.getMenuChoice(options.size());

    if (options[choice].rfind("Heal", 0) == 0)
    {
        applyHeal(party);
        renderer.renderMessage("The party rests and recovers HP.");
    }
    else if (options[choice].rfind("Purge", 0) == 0)
    {
        applyPurge(party, renderer);
    }
    else if (options[choice].rfind("Equip", 0) == 0)
    {
        applyEquip(party, renderer, input);
    }
    else if (isDraft && options[choice].rfind("Attune", 0) == 0)
    {
        applyAttune(party, renderer);
    }
    // "Continue": fall through

    (void)meta;
    (void)eventBus;
}

std::string RestNode::description() const
{
    return "[Rest] Recover HP or purge Exposure.";
}

void RestNode::applyHeal(Party &party) const
{
    for (std::size_t i{0}; i < party.size(); ++i)
    {
        Unit *u{party.getUnitAt(i)};
        if (u && u->isAlive())
        {
            const int missing{u->getMaxHp() - u->getHp()};
            u->heal(missing / 2 + 10);
        }
    }
}

void RestNode::applyPurge(Party &party, IRenderer &renderer) const
{
    for (std::size_t i{0}; i < party.size(); ++i)
    {
        Unit *u{party.getUnitAt(i)};
        if (!u)
            continue;
        if (auto *pc = dynamic_cast<PlayableCharacter *>(u))
            pc->modifyExposure(-CombatConstants::kPurgeExposureReduction);
    }
    renderer.renderMessage("Exposure reduced across the party.");
}

void RestNode::applyAttune(Party &party, IRenderer &renderer) const
{
    bool attuneAvailable{false};
    for (std::size_t i{0}; i < party.size(); ++i)
    {
        Unit *u{party.getUnitAt(i)};
        auto *pc{dynamic_cast<PlayableCharacter *>(u)};
        if (!pc)
            continue;
        const auto &slots{pc->getEquippedSkills()};
        for (int s{0}; s < EquippedSkillSet::kEquipSlots; ++s)
        {
            if (slots.slots[static_cast<std::size_t>(s)].unlocked)
            {
                attuneAvailable = true;
                renderer.renderMessage("  " + pc->getName() + " Slot " + std::to_string(s + 1) + ": unlocked (no pool skills yet)");
            }
        }
    }
    if (!attuneAvailable)
        renderer.renderMessage("No unlocked slots available to attune.");
}

void RestNode::applyEquip(Party &party,
                          IRenderer &renderer,
                          IInputHandler &input) const
{
    const auto &equipment{party.getInventory().getEquipment()};
    if (equipment.empty())
    {
        renderer.renderMessage("No equipment in inventory.");
        return;
    }

    // Build item selection menu.
    std::vector<std::string> itemOptions{};
    itemOptions.push_back("Cancel");
    for (const Item &item : equipment)
    {
        const std::string slotStr{
            item.equipSlot.has_value()
                ? (*item.equipSlot == EquipSlot::Weapon  ? "Weapon"
                   : *item.equipSlot == EquipSlot::Armor ? "Armor"
                                                         : "Accessory")
                : "Unknown"};
        itemOptions.push_back(item.name + " (" + slotStr + ", " + std::to_string(item.goldValue) + " gold)");
    }

    input.setMenuContext("SELECT ITEM TO EQUIP", itemOptions);
    renderer.renderSelectionMenu("SELECT ITEM TO EQUIP", itemOptions);
    const std::size_t itemChoice = input.getMenuChoice(itemOptions.size());

    if (itemChoice == 0)
    {
        renderer.renderMessage("Cancelled.");
        return;
    }

    const Item selectedItem{equipment[itemChoice - 1]};

    // Collect alive player characters.
    std::vector<PlayableCharacter *> pcs{};
    for (std::size_t i{0}; i < party.size(); ++i)
        if (auto *pc = dynamic_cast<PlayableCharacter *>(party.getUnitAt(i)))
            if (pc->isAlive())
                pcs.push_back(pc);

    if (pcs.empty())
    {
        renderer.renderMessage("No alive characters to equip.");
        return;
    }

    // Build character selection menu.
    std::vector<std::string> charOptions{};
    charOptions.push_back("Cancel");
    for (const PlayableCharacter *pc : pcs)
        charOptions.push_back(pc->getName());

    input.setMenuContext("EQUIP TO CHARACTER", charOptions);
    renderer.renderSelectionMenu("EQUIP TO CHARACTER", charOptions);
    const std::size_t charChoice = input.getMenuChoice(charOptions.size());

    if (charChoice == 0)
    {
        renderer.renderMessage("Cancelled.");
        return;
    }

    PlayableCharacter *target{pcs[charChoice - 1]};
    auto displaced{target->equip(selectedItem)};
    party.getInventory().removeEquipmentAt(itemChoice - 1);

    renderer.renderMessage(target->getName() + " equipped " + selectedItem.name + "!");

    if (displaced.has_value())
    {
        party.getInventory().addItem(*displaced, 1);
        renderer.renderMessage(displaced->name + " returned to inventory.");
    }
}