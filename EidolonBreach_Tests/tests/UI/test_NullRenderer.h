#pragma once
/**
 * @file test_NullRenderer.h
 * @brief No-op IRenderer for use in unit tests.
 */

#include "UI/IRenderer.h"
#include "Battle/TurnSlot.h"

class NullRenderer : public IRenderer
{
  public:
    void renderActionResult(const std::string &, const ActionResult &) override {}
    void renderBreak(const std::string &) override {}
    void renderStunned(const std::string &) override {}
    void renderVictory(const std::string &, std::optional<Drop>) override {}
    void renderDefeat(const std::string &) override {}
    void renderPartyStatus(const Party &, const Party &) override {}
    void renderMessage(const std::string &) override {}
    void renderResonanceField(const ResonanceField &) override {}
    void renderActionMenu(const PlayableCharacter &, const Party &) override {}
    void renderTargetList(const std::vector<std::string> &) override {}
    void renderTurnOrder(const std::vector<TurnSlot> &) override {}
};