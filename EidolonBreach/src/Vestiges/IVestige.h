#pragma once
/**
 * @file IVestige.h
 * @brief Interface for run-modifying Vestiges.
 *
 * Vestiges are collected during a run and lost at run end. Each hook is called
 * by Battle at the appropriate moment. Default implementations are no-ops so
 * concrete vestiges override only the hooks they need.
 *
 * Hook call sites (all in Battle):
 *   onBattleStart  — after BattleStartedEvent is emitted; use to subscribe to EventBus.
 *   onTurnStart    — at the start of each PlayableCharacter's turn, before action choice.
 *                    Not called for Enemy or Summon turns.
 *   onAction       — after IAction::execute() returns but before processActionResult().
 *                    Vestiges may modify ActionResult here (e.g. refund SP, apply bonus).
 *   onBattleEnd    — before clearBattleScope(); use to reset per-battle state.
 */

#include <string>

class Battle;
class PlayableCharacter;
struct ActionResult;
struct BattleState;

class IVestige
{
  public:
    virtual ~IVestige() = default;

    virtual void onBattleStart(Battle &, BattleState &) {}
    virtual void onTurnStart(PlayableCharacter &, BattleState &) {}
    virtual void onAction(PlayableCharacter &, ActionResult &, BattleState &) {}
    virtual void onBattleEnd(BattleState &) {}

    [[nodiscard]] virtual std::string getName() const = 0;
    [[nodiscard]] virtual std::string getDescription() const = 0;
};