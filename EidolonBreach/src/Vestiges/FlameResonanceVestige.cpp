/**
 * @file FlameResonanceVestige.cpp
 * @brief FlameResonanceVestige implementation.
 */

#include "Vestiges/FlameResonanceVestige.h"
#include "Battle/BattleState.h"
#include "Battle/ResonanceField.h"
#include "Core/ActionResult.h"
#include "Core/Affinity.h"
#include "Entities/PlayableCharacter.h"

void FlameResonanceVestige::onAction(PlayableCharacter & /*actor*/,
                                     ActionResult &result,
                                     BattleState &state)
{
    if (result.actionAffinity == Affinity::Blaze)
        state.resonanceField.addContribution(Affinity::Blaze, kBlazeBonus);
}

std::string FlameResonanceVestige::getName() const
{
    return "Flame Resonance";
}

std::string FlameResonanceVestige::getDescription() const
{
    return "Blaze actions add +5 to the Resonance Field gauge.";
}