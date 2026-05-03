/**
 * @file RestNode.cpp
 * @brief RestNode implementation.
 */

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
              << CombatConstants::kPurgeExposureReduction << "\n";
    if (isDraft)
        std::cout << "  [3] Attune - re-equip slot skills\n";
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