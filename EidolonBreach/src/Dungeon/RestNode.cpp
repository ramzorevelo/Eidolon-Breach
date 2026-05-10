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

    bool healUsed{false};
    bool purgeUsed{false};
    bool equipUsed{false};

    while (true)
    {
        // Check whether any PC is Fractured.
        bool anyFractured{false};
        for (std::size_t i{0}; i < party.size(); ++i)
        {
            Unit *u{party.getUnitAt(i)};
            auto *pc{u ? u->asPlayableCharacter() : nullptr};
            if (pc && pc->isFractured())
            {
                anyFractured = true;
                break;
            }
        }

        std::vector<std::string> options{};
        if (!healUsed)
            options.push_back("Heal — restore partial HP to all allies");
        if (!purgeUsed)
            options.push_back("Purge — reduce all Exposure by " +
                              std::to_string(CombatConstants::kPurgeExposureReduction));
        if (!equipUsed)
            options.push_back("Equip — equip items from party inventory");
        if (isDraft)
            options.push_back("Attune — re-equip slot skills");
        if (anyFractured)
            options.push_back("Stabilize — remove Fracture from one character");
        options.push_back("Continue");

        input.setMenuContext("REST SITE", options);
        renderer.renderSelectionMenu("REST SITE", options);
        const std::size_t choice = input.getMenuChoice(options.size());

        const std::string &chosen{options[choice]};

        if (chosen.rfind("Heal", 0) == 0)
        {
            applyHeal(party);
            renderer.renderMessage("The party rests and recovers HP.");
            healUsed = true;
        }
        else if (chosen.rfind("Purge", 0) == 0)
        {
            applyPurge(party, renderer);
            purgeUsed = true;
        }
        else if (chosen.rfind("Equip", 0) == 0)
        {
            applyEquip(party, renderer, input);
            equipUsed = true;
        }
        else if (isDraft && chosen.rfind("Attune", 0) == 0)
        {
            applyAttune(party, renderer);
        }
        else if (chosen.rfind("Stabilize", 0) == 0)
        {
            applyStabilize(party, renderer, input);
            break; // Stabilize consumes the node immediately.
        }
        else
        {
            break; // "Continue"
        }

        // If all repeatable options are exhausted and there are no Fractured PCs,
        // auto-exit to avoid trapping the player in an empty menu.
        const bool allUsed{healUsed && purgeUsed && equipUsed};
        if (allUsed && !anyFractured && !isDraft)
            break;
    }

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
        auto *pc{u->asPlayableCharacter()};
        if (pc)
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
        auto *pc{u->asPlayableCharacter()};
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
    {
        Unit *u{party.getUnitAt(i)};
        auto *pc{u ? u->asPlayableCharacter() : nullptr};
        if (pc && pc->isAlive())
            pcs.push_back(pc);
    }

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

void RestNode::applyStabilize(Party &party,
                              IRenderer &renderer,
                              IInputHandler &input) const
{
    // Collect Fractured PCs.
    std::vector<PlayableCharacter *> fractured{};
    for (std::size_t i{0}; i < party.size(); ++i)
    {
        Unit *u{party.getUnitAt(i)};
        auto *pc{u ? u->asPlayableCharacter() : nullptr};
        if (pc && pc->isFractured())
            fractured.push_back(pc);
    }

    if (fractured.empty())
    {
        renderer.renderMessage("No Fractured characters to stabilize.");
        return;
    }

    PlayableCharacter *target{nullptr};

    if (fractured.size() == 1)
    {
        target = fractured[0];
    }
    else
    {
        std::vector<std::string> picks{};
        for (const PlayableCharacter *pc : fractured)
            picks.push_back(pc->getName());

        input.setMenuContext("STABILIZE — CHOOSE CHARACTER", picks);
        renderer.renderSelectionMenu("STABILIZE — CHOOSE CHARACTER", picks);
        const std::size_t pick = input.getMenuChoice(picks.size());
        target = fractured[pick];
    }

    target->clearFracture();
    renderer.renderMessage(target->getName() + "'s Fracture has been stabilized.");
}