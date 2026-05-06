#pragma once
/**
 * @file SDL3Renderer.h
 * @brief SDL3 implementation of IRenderer. Owns the SDL_Window and SDL_Renderer.
 *        Implements all IRenderer methods; rendering primitives are in SDL3Renderer.cpp.
 *        Instantiated once in main(); injected into Battle and Dungeon by reference.
 */

#include "UI/IRenderer.h"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>
#include <vector>

class SDL3Renderer : public IRenderer
{
  public:
    /**
     * @brief Initialize SDL3, create window and renderer.
     * @param windowTitle Title shown in the OS window frame.
     * @param width       Initial window width in pixels.
     * @param height      Initial window height in pixels.
     * @throws std::runtime_error if SDL_Init, window, or renderer creation fails.
     */
    SDL3Renderer(const char *windowTitle, int width, int height);

    /** @brief Destroy renderer, window, and shut down SDL3 and TTF subsystems. */
    ~SDL3Renderer() override;

    // Non-copyable, non-movable — owns SDL handles.
    SDL3Renderer(const SDL3Renderer &) = delete;
    SDL3Renderer &operator=(const SDL3Renderer &) = delete;
    SDL3Renderer(SDL3Renderer &&) = delete;
    SDL3Renderer &operator=(SDL3Renderer &&) = delete;

    // IRenderer interface 
    void renderActionResult(const std::string &actorName,
                            const ActionResult &result) override;
    void renderBreak(const std::string &enemyName) override;
    void renderStunned(const std::string &enemyName) override;
    void renderVictory(const std::string &enemyName, std::optional<Drop> drop) override;
    void renderDefeat(const std::string &playerName) override;
    void renderPartyStatus(const Party &playerParty, const Party &enemyParty) override;
    void renderMessage(const std::string &message) override;
    void renderResonanceField(const ResonanceField &field) override;
    void renderActionMenu(const PlayableCharacter &character, const Party &party) override;
    void renderTargetList(const std::vector<std::string> &names) override;
    void renderTurnOrder(const std::vector<TurnSlot> &order) override;

  private:
    SDL_Window *m_window{nullptr};
    SDL_Renderer *m_renderer{nullptr};
    TTF_Font *m_font{nullptr};

    int m_windowWidth{};
    int m_windowHeight{};

    // Panel layout rects. Computed once in computePanelLayout, reused each frame.
    SDL_FRect m_turnOrderPanel{};
    SDL_FRect m_playerPanel{};
    SDL_FRect m_playerForm{};
    SDL_FRect m_centerPanel{};
    SDL_FRect m_enemyForm{};
    SDL_FRect m_enemyPanel{};
    SDL_FRect m_actionMenu{};
    SDL_FRect m_logPanel{};
    SDL_FRect m_hintBar{};

    // Action log: Stores the last N messages for scrollable display.
    static constexpr int kMaxLogLines{64};
    std::vector<std::string> m_log{};
    int m_logScrollOffset{0};

    /**
     * @brief Compute proportional panel rects from the current window dimensions.
     *        Called in the constructor and on SDL_EVENT_WINDOW_RESIZED.
     */
    void computePanelLayout(int w, int h);

    /** @brief Clear the backbuffer to the background color. */
    void beginFrame();

    /** @brief Present the completed frame. */
    void endFrame();

    /**
     * @brief Render a filled rectangle using the given RGBA color.
     * @param rect  Region to fill.
     * @param r,g,b,a  Color components (0–255).
     */
    void fillRect(const SDL_FRect &rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255);

    /**
     * @brief Render a horizontal progress bar.
     * @param rect      Total bar region.
     * @param fraction  Filled portion [0.0, 1.0].
     * @param fr,fg,fb  Fill color.
     * @param br,bg,bb  Background (empty) color.
     */
    void renderBar(const SDL_FRect &rect, float fraction,
                   Uint8 fr, Uint8 fg, Uint8 fb,
                   Uint8 br, Uint8 bg, Uint8 bb);

    /**
     * @brief Render UTF-8 text at the given position.
     * @param text    String to render.
     * @param x, y   Top-left pixel position.
     * @param r,g,b  Text color.
     */
    void renderText(const std::string &text, float x, float y,
                    Uint8 r, Uint8 g, Uint8 b);
};