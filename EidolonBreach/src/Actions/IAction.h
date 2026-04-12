#pragma once
#include "Core/ActionResult.h"
#include "Core/Affinity.h"
#include "Core/TargetInfo.h"
#include <optional>
#include <string>

class PlayableCharacter;  
class Party;              

/**
 * @file IAction.h
 * @brief Interface for all player‑initiated combat actions.
 */

/** Player action (basic attack, skill, ultimate, item use). */
class IAction
{
public:
    virtual ~IAction() = default;

    virtual std::string label() const = 0;

    /** Perform the action. 
     * The action is responsible for resource changes 
     * and applying effects/damage to the target. 
     */
    virtual ActionResult execute(PlayableCharacter& user,
        Party& allies,
        Party& enemies,
        std::optional<TargetInfo>   target) = 0;

    virtual bool isAvailable(const PlayableCharacter&) const { return true; }

    /** @return The affinity of this action (default Aether). 
     * Used by Resonance Field in Phase 3. 
     */
    virtual Affinity getAffinity() const { return Affinity::Aether; }
};