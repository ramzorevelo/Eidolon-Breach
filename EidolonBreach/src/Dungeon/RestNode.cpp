/**
 * @file RestNode.cpp
 * @brief RestNode implementation.
 */

#include "Dungeon/RestNode.h"
#include "Core/CombatConstants.h"
#include "Entities/Party.h"
#include "Entities/PlayableCharacter.h"
#include "Entities/Unit.h"
#include <iostream>

void RestNode::enter(Party &party,
                     MetaProgress & /*meta*/,
                     RunContext & /*runCtx*/,
                     EventBus & /*eventBus*/)
{
    std::cout << "\n=== REST SITE ===\n"
              << "  [1] Heal - restore partial HP to all allies\n"
              << "  [2] Purge - reduce all Exposure by "
              << CombatConstants::kPurgeExposureReduction << "\n"
              << "  [3] Continue\n"
              << "Choose: ";

    int choice{3};
    std::cin >> choice;

    switch (choice)
    {
    case 1:
        applyHeal(party);
        break;
    case 2:
        applyPurge(party);
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