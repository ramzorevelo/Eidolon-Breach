/**
 * @file TreasureNode.cpp
 * @brief TreasureNode implementation.
 */

#include "Dungeon/TreasureNode.h"
#include "Dungeon/VestigeOffer.h"
#include "Entities/Party.h"
#include "Vestiges/VestigeFactory.h"
#include "UI/IRenderer.h"
#include "UI/IInputHandler.h"
#include <random>
#include <limits>

TreasureNode::TreasureNode(int goldAmount, std::uint32_t vestigeRngSeed)
    : m_goldAmount{goldAmount}, m_vestigeRngSeed{vestigeRngSeed}
{
}

void TreasureNode::enter(Party &party, MetaProgress & /*meta*/,
                         RunContext & /*runCtx*/, EventBus & /*eventBus*/,
                         IRenderer &renderer, IInputHandler &input)
{
    party.getInventory().gold += m_goldAmount;
    renderer.renderMessage("=== TREASURE ===");
    renderer.renderMessage("Found " + std::to_string(m_goldAmount) + " gold. Party gold: " + std::to_string(party.getInventory().gold));

    std::mt19937 rng{m_vestigeRngSeed};
    auto vestige = VestigeFactory::makeRandom(VestigeFactory::Rarity::Common, rng);
    renderer.renderMessage("A vestige resonates: [" + vestige->getName() + "] — " + vestige->getDescription());
    renderer.presentPause(400);

    auto overflow = party.addVestige(std::move(vestige));
    if (overflow.has_value())
        DungeonHelpers::offerVestigeDiscard(party, std::move(*overflow), renderer, input);
}

std::string TreasureNode::description() const
{
    return "[Treasure] Gold and a vestige await.";
}