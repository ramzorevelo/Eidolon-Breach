#pragma once
/**
 * @file SDL3Renderer.h
 * @brief SDL3 implementation of IRenderer.
 *
 * Design: each render*() call caches the data it receives, then calls
 * redrawAll(). redrawAll() is the only place that calls beginFrame/endFrame —
 * it clears the backbuffer, redraws every panel from cached state, and
 * presents once. This guarantees all panels are always visible regardless
 * of which render method was called last.
 */

#include "UI/IRenderer.h"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>
#include <vector>

struct TurnSlot;
class Party;
class PlayableCharacter;
class ResonanceField;
enum class Affinity;

class SDL3Renderer : public IRenderer
{
  public:
    SDL3Renderer(const char *windowTitle, int width, int height);
    ~SDL3Renderer() override;

    SDL3Renderer(const SDL3Renderer &) = delete;
    SDL3Renderer &operator=(const SDL3Renderer &) = delete;
    SDL3Renderer(SDL3Renderer &&) = delete;
    SDL3Renderer &operator=(SDL3Renderer &&) = delete;

    // IRenderer 
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
    void renderTargetList(const std::vector<std::string> &names,
                          bool isAllyTarget = false) override;
    void renderTurnOrder(const std::vector<TurnSlot> &order) override;
    void renderHintBar(const std::string &hint) override;
    void clearTargetHighlight() override;
    void updateTargetHighlight(int index) override;
    void presentPause(int ms) override;
    void renderSelectionMenu(const std::string &title,
                             const std::vector<std::string> &options,
                             std::size_t selected = 0) override;
    void clearBattleCache();
    void setLogScrollOffset(int delta);
    void expandLog(bool expand);
    [[nodiscard]] bool isLogScrollable() const;
    [[nodiscard]] int getActionRowAt(int x, int y) const;
    [[nodiscard]] int getUnitCardAt(int x, int y, bool isPlayerSide) const;
    void renderTooltip(const std::string &name, float hpFraction,
                       const std::string &effectSummary, int screenX, int screenY);
    [[nodiscard]] bool isHighlightingEnemies() const
    {
        return m_highlightingEnemies;
    }
    [[nodiscard]] int getWindowHeight() const
    {
        return m_windowHeight;
    }
  private:
    // SDL handles 
    SDL_Window *m_window{nullptr};
    SDL_Renderer *m_renderer{nullptr};
    TTF_Font *m_font{nullptr};

    int m_windowWidth{};
    int m_windowHeight{};

    // Panel layout rects 
    SDL_FRect m_turnOrderPanel{};
    SDL_FRect m_playerPanel{};
    SDL_FRect m_centerPanel{};
    SDL_FRect m_enemyPanel{};
    SDL_FRect m_actionMenu{};
    SDL_FRect m_logPanel{};
    SDL_FRect m_hintBar{};

    // Cached render state 
    // Pointers are non-owning observers. They are valid for the duration of
    // a Battle::run() call since Party/ResonanceField outlive all render calls.
    const Party *m_cachedPlayerParty{nullptr};
    const Party *m_cachedEnemyParty{nullptr};
    const PlayableCharacter *m_cachedActiveCharacter{nullptr};
    const Party *m_cachedActiveParty{nullptr};
    const ResonanceField *m_cachedResonanceField{nullptr};
    std::vector<TurnSlot> m_cachedTurnOrder{};
    std::string m_cachedHint{};

    // Action log 
    static constexpr int kMaxLogLines{64};
    std::vector<std::string> m_log{};
    int m_logScrollOffset{0};
    bool m_logExpanded{false};
    Uint64 m_lastLogTime{0};

    // RF trigger overlay state.
    Affinity m_rfTriggerAffinity{};
    Uint64 m_rfTriggerExpiry{0};
    int m_cachedRfGauge{0};

    // Core render loop 
    /**
     * @brief Clear the backbuffer, draw every panel from cached state, present.
     *        This is the only function that calls SDL_RenderClear / SDL_RenderPresent.
     */
    void redrawAll();

    // Per-panel draw helpers (called only from redrawAll) 
    void drawTurnOrderStrip();
    void drawPlayerPanel();
    void drawEnemyPanel();
    void drawCenterPanel();
    void drawActionMenuPanel();
    void drawLogPanel();
    void drawHintBarPanel();

    // Layout 
    void computePanelLayout(int w, int h);

    // Low-level drawing primitives
    void fillRect(const SDL_FRect &rect,
                  Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255);

    void renderBar(const SDL_FRect &rect, float fraction,
                   Uint8 fr, Uint8 fg, Uint8 fb,
                   Uint8 br, Uint8 bg, Uint8 bb);

    void renderText(const std::string &text, float x, float y,
                    Uint8 r, Uint8 g, Uint8 b);

    int m_highlightedTargetIndex{-1}; // -1 = no active targeting
    bool m_highlightingEnemies{true}; // true = enemy panel, false = player panel
    static SDL_Color affinityColor(Affinity a);
    TTF_Font *m_fontLarge{nullptr};   // used for full-screen selection menus
    void renderTextEx(TTF_Font *font, const std::string &text,
                      float x, float y, Uint8 r, Uint8 g, Uint8 b);
};