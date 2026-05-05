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
    .affinity = Affinity::Aether,
    .category = ActionCategory::Vent,
};

std::string VentAction::label() const
{
    return "Vent (Exposure -> 0, ends turn)";
}

bool VentAction::isAvailable(const PlayableCharacter &user, const Party & /*party*/) const
{
    return user.canVent();
}

ActionResult VentAction::execute(PlayableCharacter &user,
                                 Party & /*allies*/,
                                 Party & /*enemies*/,
                                 std::optional<TargetInfo> /*target*/)
{
    // Capture state before clearing. Consolation requires Exposure >= 50.
    const bool consolation{user.getExposure() >= PlayableCharacter::kVentThreshold50};

    // Surging proc is cancelled when venting at 50–99.
    // (isSurgingProcArmed is false if Exposure never crossed 75, safe to call unconditionally.)
    if (consolation && user.isSurgingProcArmed())
        user.consumeSurgingProc();

    user.modifyExposure(-user.getExposure()); // clamp to 0

    ActionResult result{ActionResult::Type::Skip, 0};
    result.ventConsolation = consolation;
    return result;
}

Affinity VentAction::getAffinity() const
{
    return Affinity::Aether;
}

const ActionData &VentAction::getActionData() const
{
    return kVentData;
}