#pragma once
/**
 * @file SummonAction.h
 * @brief Describes one auto-executed action in a Summon's action pool.
 *        Uses std::function rather than IAction because IAction::execute()
 *        requires a PlayableCharacter& that Summons do not have.
 */

#include "Core/ActionResult.h"
#include <functional>
#include <string>

class Summon;
class Party;
struct BattleState;

struct SummonAction
{
    std::string label{};
    std::function<ActionResult(Summon &, Party &allies, Party &enemies, BattleState &)> execute{};
};