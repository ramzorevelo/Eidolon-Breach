#pragma once
/**
 * @file VestigeOffer.h
 * @brief Shared helper for offering a vestige when the party is at capacity.
 *        Used by EliteNode and TreasureNode.
 */
#include <memory>

class Party;
class IRenderer;
class IInputHandler;
class IVestige;

namespace DungeonHelpers
{
/**
 * @brief Present a discard menu when the party's vestige slots are full.
 *        The player chooses to either discard the incoming vestige or
 *        replace one of their current vestiges with it.
 * @param party     The player party.
 * @param incoming  The new vestige being offered.
 * @param renderer  For displaying the menu.
 * @param input     For reading the player's choice.
 */
void offerVestigeDiscard(Party &party,
                         std::unique_ptr<IVestige> incoming,
                         IRenderer &renderer,
                         IInputHandler &input);
} // namespace DungeonHelpers