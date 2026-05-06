/**
 * @file SDL3Renderer.cpp
 * @brief SDL3Renderer stub. All IRenderer methods are no-ops pending implementation.
 *        Constructor/destructor are fully functional.
 */

#include "UI/SDL3Renderer.h"
#include "Battle/ResonanceField.h"
#include "Battle/TurnSlot.h"
#include "Core/ActionResult.h"
#include "Core/Drop.h"
#include "Entities/Party.h"
#include "Entities/PlayableCharacter.h"
#include <stdexcept>

SDL3Renderer::SDL3Renderer(const char *windowTitle, int width, int height)
    : m_windowWidth{width}, m_windowHeight{height}
{
    if (!SDL_Init(SDL_INIT_VIDEO))
        throw std::runtime_error{std::string{"SDL3Renderer: SDL_Init failed: "} + SDL_GetError()};

    if (!TTF_Init())
        throw std::runtime_error{std::string{"SDL3Renderer: TTF_Init failed: "} + SDL_GetError()};

    m_window = SDL_CreateWindow(windowTitle, width, height, SDL_WINDOW_RESIZABLE);
    if (!m_window)
        throw std::runtime_error{std::string{"SDL3Renderer: CreateWindow failed: "} + SDL_GetError()};

    m_renderer = SDL_CreateRenderer(m_window, nullptr);
    if (!m_renderer)
        throw std::runtime_error{std::string{"SDL3Renderer: CreateRenderer failed: "} + SDL_GetError()};

    computePanelLayout(width, height);
}

SDL3Renderer::~SDL3Renderer()
{
    if (m_font)
    {
        TTF_CloseFont(m_font);
        m_font = nullptr;
    }
    if (m_renderer)
    {
        SDL_DestroyRenderer(m_renderer);
        m_renderer = nullptr;
    }
    if (m_window)
    {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }
    TTF_Quit();
    SDL_Quit();
}

// Layout 

void SDL3Renderer::computePanelLayout(int w, int h)
{
    const float fw = static_cast<float>(w);
    const float fh = static_cast<float>(h);
    const float turnH = fh * 0.10f;
    const float logH = fh * 0.15f;
    const float hintH = 28.f;
    const float mainH = fh - turnH - logH - hintH;
    const float menuW = fw * 0.35f;
    const float logW = fw - menuW;

    m_turnOrderPanel = {0.f, 0.f, fw, turnH};
    m_playerPanel = {0.f, turnH, fw * 0.18f, mainH};
    m_playerForm = {fw * 0.18f, turnH, fw * 0.22f, mainH};
    m_centerPanel = {fw * 0.40f, turnH, fw * 0.06f, mainH};
    m_enemyForm = {fw * 0.46f, turnH, fw * 0.22f, mainH};
    m_enemyPanel = {fw * 0.68f, turnH, fw * 0.32f, mainH};
    m_actionMenu = {0.f, turnH + mainH, menuW, logH};
    m_logPanel = {menuW, turnH + mainH, logW, logH};
    m_hintBar = {0.f, fh - hintH, fw, hintH};
}

// Frame helpers 

void SDL3Renderer::beginFrame()
{
    SDL_SetRenderDrawColor(m_renderer, 18, 18, 24, 255);
    SDL_RenderClear(m_renderer);
}

void SDL3Renderer::endFrame()
{
    SDL_RenderPresent(m_renderer);
}

// Drawing helpers 

void SDL3Renderer::fillRect(const SDL_FRect &rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    SDL_SetRenderDrawColor(m_renderer, r, g, b, a);
    SDL_RenderFillRect(m_renderer, &rect);
}

void SDL3Renderer::renderBar(const SDL_FRect &rect, float fraction,
                             Uint8 fr, Uint8 fg, Uint8 fb,
                             Uint8 br, Uint8 bg, Uint8 bb)
{
    // Background (empty portion).
    fillRect(rect, br, bg, bb);

    // Filled portion: clamp fraction to [0, 1].
    const float clamped = (fraction < 0.f) ? 0.f : (fraction > 1.f ? 1.f : fraction);
    if (clamped > 0.f)
    {
        const SDL_FRect filled{rect.x, rect.y, rect.w * clamped, rect.h};
        fillRect(filled, fr, fg, fb);
    }
}

void SDL3Renderer::renderText(const std::string &text, float x, float y,
                              Uint8 r, Uint8 g, Uint8 b)
{
    if (!m_font || text.empty())
        return;

    const SDL_Color color{r, g, b, 255};
    SDL_Surface *surface = TTF_RenderText_Blended(m_font, text.c_str(), 0, color);
    if (!surface)
        return;

    SDL_Texture *texture = SDL_CreateTextureFromSurface(m_renderer, surface);
    SDL_DestroySurface(surface);
    if (!texture)
        return;

    float tw{}, th{};
    SDL_GetTextureSize(texture, &tw, &th);
    const SDL_FRect dst{x, y, tw, th};
    SDL_RenderTexture(m_renderer, texture, nullptr, &dst);
    SDL_DestroyTexture(texture);
}

// IRenderer stubs 

void SDL3Renderer::renderActionResult(const std::string &, const ActionResult &) {}
void SDL3Renderer::renderBreak(const std::string &) {}
void SDL3Renderer::renderStunned(const std::string &) {}
void SDL3Renderer::renderVictory(const std::string &, std::optional<Drop>) {}
void SDL3Renderer::renderDefeat(const std::string &) {}
void SDL3Renderer::renderPartyStatus(const Party &, const Party &) {}
void SDL3Renderer::renderMessage(const std::string &) {}
void SDL3Renderer::renderResonanceField(const ResonanceField &) {}
void SDL3Renderer::renderActionMenu(const PlayableCharacter &, const Party &) {}
void SDL3Renderer::renderTargetList(const std::vector<std::string> &) {}
void SDL3Renderer::renderTurnOrder(const std::vector<TurnSlot> &) {}