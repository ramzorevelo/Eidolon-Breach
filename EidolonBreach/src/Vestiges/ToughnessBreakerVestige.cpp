/**
 * @file ToughnessBreakerVestige.cpp
 * @brief ToughnessBreakerVestige implementation.
 */

#include "Battle/BattleState.h"
#include "Core/ActionResult.h"
#include "Entities/Party.h"
#include "Entities/PlayableCharacter.h"
#include "Entities/Unit.h"
#include "Vestiges/ToughnessBreakerVestige.h"

void ToughnessBreakerVestige::onAction(PlayableCharacter & /*actor*/,
                                       ActionResult &result,
                                       BattleState &state)
{
    if (result.toughnessDamage <= 0 || result.targetEnemyIndex < 0)
        return;
    if (state.enemyParty == nullptr)
        return;

    Unit *target{state.enemyParty->getUnitAt(
        static_cast<std::size_t>(result.targetEnemyIndex))};
    if (!target || !target->isAlive())
        return;

    // Only apply the bonus when the action affinity is a weakness of the target.
    if (target->getToughnessAffinityModifier(result.actionAffinity) <= kWeaknessThreshold)
        return;

    const int bonus{static_cast<int>(
        static_cast<float>(result.toughnessDamage) * kBonusFraction)};
    if (bonus > 0)
        target->applyToughnessHit(bonus, result.actionAffinity);
}

std::string ToughnessBreakerVestige::getName() const
{
    return "Toughness Breaker";
}

std::string ToughnessBreakerVestige::getDescription() const
{
    return "+20% Toughness damage against enemies weak to your action's affinity.";
}