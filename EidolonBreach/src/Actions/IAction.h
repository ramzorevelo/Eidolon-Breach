#pragma once

/**
 * @file IAction.h
 * @brief Interface for all player‑initiated combat actions.
 */

#include "Core/ActionResult.h"
#include "Core/Affinity.h"
#include "Core/TargetInfo.h"
#include <optional>
#include <string>

class PlayableCharacter;
class Party;


/** Player action (basic attack, skill, ultimate, item use). */
class IAction
{
  public:
    virtual ~IAction() = default;

    virtual std::string label() const = 0;

    /** Perform the action.
     * The action is responsible for resource changes and applying effects/damage.
     */
    virtual ActionResult execute(PlayableCharacter &user,
                                 Party &allies,
                                 Party &enemies,
                                 std::optional<TargetInfo> target) = 0;

    /** @return true if the user meets resource requirements. */
    virtual bool isAvailable(const PlayableCharacter &user, const Party &party) const
    {
        (void)user;
        (void)party;
        return true;
    }

    /** @return The affinity of this action (default Aether). */
    virtual Affinity getAffinity() const
    {
        return Affinity::Aether;
    }
};