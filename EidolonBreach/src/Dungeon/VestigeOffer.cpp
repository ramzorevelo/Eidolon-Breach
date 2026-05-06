/**
 * @file VestigeOffer.cpp
 * @brief DungeonHelpers::offerVestigeDiscard implementation.
 */
#include "Dungeon/VestigeOffer.h"
#include "Entities/Party.h"
#include "UI/IInputHandler.h"
#include "UI/IRenderer.h"
#include "Vestiges/IVestige.h"

namespace DungeonHelpers
{
void offerVestigeDiscard(Party &party,
                         std::unique_ptr<IVestige> incoming,
                         IRenderer &renderer,
                         IInputHandler &input)
{
    std::vector<std::string> options{};
    options.push_back("Decline — discard " + incoming->getName());
    for (const auto &v : party.getVestiges())
        options.push_back("Replace: " + v->getName() + " — " + v->getDescription());

    const std::string title = "VESTIGE FULL: " + incoming->getName() + " — " + incoming->getDescription();

    input.setMenuContext(title, options);
    renderer.renderSelectionMenu(title, options);
    const std::size_t choice = input.getMenuChoice(options.size());

    if (choice == 0)
    {
        renderer.renderMessage(incoming->getName() + " discarded.");
        return;
    }

    std::ignore = party.removeVestige(choice - 1);
    std::ignore = party.addVestige(std::move(incoming));
    renderer.renderMessage("Vestige replaced.");
}
} // namespace DungeonHelpers