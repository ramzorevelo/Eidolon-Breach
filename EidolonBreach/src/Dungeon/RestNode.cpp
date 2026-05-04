/**
 * @file RestNode.cpp
 * @brief RestNode implementation.
 */
#include "Entities/PlayableCharacter.h"
#include "Items/Item.h"
#include <limits>
#include "Dungeon/RestNode.h"
#include "Core/CombatConstants.h"
#include "Entities/Party.h"
#include "Entities/PlayableCharacter.h"
#include "Entities/Unit.h"
#include "Actions/SlotState.h"
#include <iostream>
#include <Core/RunContext.h>

void RestNode::enter(Party &party,
                     MetaProgress & /*meta*/,
                     RunContext &runCtx,
                     EventBus & /*eventBus*/)
{
    const bool isDraft{runCtx.runMode == RunMode::EidolonLabyrinth};
    std::cout << "\n=== REST SITE ===\n"
              << "  [1] Heal - restore partial HP to all allies\n"
              << "  [2] Purge - reduce all Exposure by "
              << CombatConstants::kPurgeExposureReduction << "\n"
              << "  [3] Equip - equip items from party inventory\n";
    if (isDraft)
        std::cout << "  [4] Attune - re-equip slot skills\n";
    std::cout << "  [0] Continue\n"
              << "Choose: ";

    int choice{0};
    std::cin >> choice;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    switch (choice)
    {
    case 1:
        applyHeal(party);
        break;
    case 2:
        applyPurge(party);
        break;
    case 3:
        applyEquip(party);
        break;
    case 4:
        if (isDraft)
            applyAttune(party);
        break;
    default:
        break;
    }
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
            u->heal(missing / 2 + 10); // 50% missing + flat 10 floor
        }
    }
    std::cout << "The party rests and recovers HP.\n";
}

void RestNode::applyPurge(Party &party) const
{
    for (std::size_t i{0}; i < party.size(); ++i)
    {
        Unit *u{party.getUnitAt(i)};
        if (!u)
            continue;
        if (auto *pc = dynamic_cast<PlayableCharacter *>(u))
            pc->modifyExposure(-CombatConstants::kPurgeExposureReduction);
    }
    std::cout << "Exposure reduced across the party.\n";
}
void RestNode::applyAttune(Party &party) const
{
    // Attune: for each PC with an unlocked slot, offer to equip an ability
    // from their ability pool. Currently ability pools only contain basic/arch/ultimate;
    // slot skills will be added per-character in a later content pass.
    std::cout << "\n  [Attune] Slot skill re-equip:\n";
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
                std::cout << "    " << pc->getName()
                          << " Slot " << (s + 1) << ": unlocked (no pool skills yet)\n";
            }
        }
    }
    if (!attuneAvailable)
        std::cout << "  No unlocked slots available to attune.\n";
}

void RestNode::applyEquip(Party &party) const
{
    const auto &equipment{party.getInventory().getEquipment()};
    if (equipment.empty())
    {
        std::cout << "  No equipment in inventory.\n";
        return;
    }

    std::cout << "\n  Equipment available:\n";
    for (std::size_t i{0}; i < equipment.size(); ++i)
    {
        const Item &item{equipment[i]};
        const std::string slotStr{
            item.equipSlot.has_value()
                ? (*item.equipSlot == EquipSlot::Weapon
                       ? "Weapon"
                   : *item.equipSlot == EquipSlot::Armor ? "Armor"
                                                         : "Accessory")
                : "Unknown"};
        std::cout << "  [" << (i + 1) << "] " << item.name
                  << " (" << slotStr << ", " << item.goldValue << " gold)\n";
    }
    std::cout << "  [0] Cancel\n"
              << "  Item: ";

    int itemChoice{0};
    std::cin >> itemChoice;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    if (itemChoice < 1 || itemChoice > static_cast<int>(equipment.size()))
    {
        std::cout << "  Cancelled.\n";
        return;
    }

    const Item selectedItem{equipment[static_cast<std::size_t>(itemChoice - 1)]};

    // Collect alive player characters only.
    std::vector<PlayableCharacter *> pcs{};
    for (std::size_t i{0}; i < party.size(); ++i)
        if (auto *pc = dynamic_cast<PlayableCharacter *>(party.getUnitAt(i)))
            if (pc->isAlive())
                pcs.push_back(pc);

    if (pcs.empty())
    {
        std::cout << "  No alive characters to equip.\n";
        return;
    }

    std::cout << "  Equip to:\n";
    for (std::size_t i{0}; i < pcs.size(); ++i)
        std::cout << "  [" << (i + 1) << "] " << pcs[i]->getName() << '\n';
    std::cout << "  [0] Cancel\n"
              << "  Character: ";

    int charChoice{0};
    std::cin >> charChoice;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    if (charChoice < 1 || charChoice > static_cast<int>(pcs.size()))
    {
        std::cout << "  Cancelled.\n";
        return;
    }

    PlayableCharacter *target{pcs[static_cast<std::size_t>(charChoice - 1)]};
    auto displaced{target->equip(selectedItem)};

    party.getInventory().removeEquipmentAt(
        static_cast<std::size_t>(itemChoice - 1));

    std::cout << "  " << target->getName() << " equipped " << selectedItem.name << "!\n";

    // Return displaced item (if any) to inventory.
    if (displaced.has_value())
    {
        party.getInventory().addItem(*displaced, 1);
        std::cout << "  " << displaced->name << " returned to inventory.\n";
    }
}