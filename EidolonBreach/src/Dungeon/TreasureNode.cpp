/**
 * @file TreasureNode.cpp
 * @brief TreasureNode implementation.
 */

#include "Dungeon/TreasureNode.h"
#include "Entities/Party.h"
#include <iostream>

TreasureNode::TreasureNode(int goldAmount)
    : m_goldAmount{goldAmount}
{
}

void TreasureNode::enter(Party &party,
                         MetaProgress & /*meta*/,
                         RunContext & /*runCtx*/,
                         EventBus & /*eventBus*/)
{
    party.getInventory().gold += m_goldAmount;
    std::cout << "\n=== TREASURE ===\n"
              << "Found " << m_goldAmount << " gold. "
              << "Party gold: " << party.getInventory().gold << '\n';
}

std::string TreasureNode::description() const
{
    return "[Treasure] Gold and supplies.";
}