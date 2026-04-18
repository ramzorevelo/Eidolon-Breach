/**
 * @file VentAction.cpp
 * @brief VentAction implementation.
 */

#include "Actions/VentAction.h"
#include "Core/ActionData.h"
#include "Entities/PlayableCharacter.h"

const ActionData VentAction::kVentData{
    .skillPower = 0.0f,
    .scaling = ScalingStat::ATK,
    .spCost = 0,
    .momentumCost = 0,
    .momentumGain = 0,
    .toughnessDamage = 0,
    .targetMode = TargetMode::Self,
    .affinity = Affinity::Aether};

std::string VentAction::label() const
{
    return "Vent (Exposure -> 0, ends turn)";
}

bool VentAction::isAvailable(const PlayableCharacter & /*user*/, const Party & /*party*/) const
{
    // Full check (Exposure > 0 && Exposure < 100) wired in Commit 10.
    return false;
}

ActionResult VentAction::execute(PlayableCharacter & /*user*/,
                                 Party & /*allies*/,
                                 Party & /*enemies*/,
                                 std::optional<TargetInfo> /*target*/)
{
    // Full Exposure reduction wired in Commit 10.
    return ActionResult{ActionResult::Type::Skip, 0};
}

Affinity VentAction::getAffinity() const
{
    return Affinity::Aether;
}

const ActionData &VentAction::getActionData() const
{
    return kVentData;
}