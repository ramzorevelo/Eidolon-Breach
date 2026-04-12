#pragma once
#include "Core/ActionResult.h"
#include "Core/Affinity.h"
#include "Core/TargetInfo.h"
#include <optional>
#include <string>

class PlayableCharacter;   // forward declaration
class Party;               // forward declaration

class IAction
{
public:
    virtual ~IAction() = default;

    virtual std::string label() const = 0;

    // Execute the action.
    // The action is responsible for:
    //   - applying HP damage to the target via Party::getUnitAt()
    //   - applying toughness hits via Unit::applyToughnessHit()
    //   - managing the user's SP / Energy
    // Returns ActionResult for Battle to render.
    virtual ActionResult execute(PlayableCharacter& user,
        Party& allies,
        Party& enemies,
        std::optional<TargetInfo>   target) = 0;

    virtual bool isAvailable(const PlayableCharacter&) const { return true; }

    // Phase 2 will use this to record affinity votes for the Resonance Field.
    virtual Affinity getAffinity() const { return Affinity::Aether; }
};