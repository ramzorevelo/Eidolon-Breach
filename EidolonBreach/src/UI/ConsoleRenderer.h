#pragma once
/**
 * @file ConsoleRenderer.h
 * @brief Console implementation of IRenderer using std::cout (Phases 0–2 placeholder).
 */

#include "UI/IRenderer.h"
#include <optional>
#include "Battle/TurnSlot.h"
#include <string>

class Party;
class Unit;
class ResonanceField;

/** Scrolling console renderer. Replaced by SDL3Renderer in Phase 3+. */
class ConsoleRenderer : public IRenderer
{
  public:
    void renderActionResult(const std::string &actorName,
                            const ActionResult &result) override;
    void renderBreak(const std::string &enemyName) override;
    void renderStunned(const std::string &enemyName) override;
    void renderVictory(const std::string &enemyName,
                       std::optional<Drop> drop) override;
    void renderDefeat(const std::string &playerName) override;
    void renderPartyStatus(const Party &playerParty,
                           const Party &enemyParty) override;
    void renderMessage(const std::string &message) override;
    void renderResonanceField(const ResonanceField &field) override;
    void renderActionMenu(const PlayableCharacter &character,
                          const Party &party) override;

    void renderTargetList(const std::vector<std::string> &names) override;
    void renderTurnOrder(const std::vector<TurnSlot> &order) override;
  private:
    static void printBar(int current, int maximum, int width = 20);
    static void renderParty(const Party &party,
                            const std::string &prefix,
                            bool showToughness);
    static void renderUnit(const Unit *unit,
                           const std::string &label,
                           bool showToughness);
    static void renderEffects(const Unit &unit);
};