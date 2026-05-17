/**
 * @file SDL3Renderer.cpp
 * @brief Full implementation of SDL3Renderer.
 *
 * All coordinates are in logical 1280×720.
 * SDL_SetRenderLogicalPresentation handles scaling.
 */

#include "UI/SDL3Renderer.h"
#include "Battle/ResonanceField.h"
#include "Battle/TurnSlot.h"
#include "Core/ActionData.h"
#include "Core/ActionResult.h"
#include "Core/Affinity.h"
#include "Core/Drop.h"
#include "Entities/Enemy.h"
#include "Entities/Party.h"
#include "Entities/PlayableCharacter.h"
#include <algorithm>
#include <array>
#include <stdexcept>

namespace
{

std::string truncate(const std::string &s, std::size_t maxChars)
{
    if (s.size() <= maxChars)
        return s;
    return s.substr(0, maxChars - 1) + "~";
}

constexpr Uint8 toUint8(int v) noexcept
{
    return static_cast<Uint8>(std::clamp(v, 0, 255));
}

} // namespace

SDL_Color SDL3Renderer::affinityColor(Affinity a)
{
    switch (a)
    {
    case Affinity::Blaze:
        return {232, 83, 26, 255};
    case Affinity::Frost:
        return {79, 195, 232, 255};
    case Affinity::Tempest:
        return {168, 232, 50, 255};
    case Affinity::Terra:
        return {200, 160, 50, 255};
    case Affinity::Aether:
        return {192, 150, 232, 255};
    default:
        return {180, 180, 180, 255};
    }
}

SDL3Renderer::SDL3Renderer(const char *windowTitle, int initialWidth, int initialHeight)
    : m_windowWidth{initialWidth}, m_windowHeight{initialHeight}
{
    if (!SDL_Init(SDL_INIT_VIDEO))
        throw std::runtime_error{"SDL3Renderer: SDL_Init failed: " + std::string{SDL_GetError()}};

    if (!TTF_Init())
        throw std::runtime_error{"SDL3Renderer: TTF_Init failed: " + std::string{SDL_GetError()}};

    m_window = SDL_CreateWindow(windowTitle, initialWidth, initialHeight,
                                SDL_WINDOW_RESIZABLE);
    if (!m_window)
        throw std::runtime_error{"SDL3Renderer: CreateWindow failed: " + std::string{SDL_GetError()}};

    m_renderer = SDL_CreateRenderer(m_window, nullptr);
    if (!m_renderer)
        throw std::runtime_error{"SDL3Renderer: CreateRenderer failed: " + std::string{SDL_GetError()}};

    SDL_SetRenderLogicalPresentation(m_renderer, 1280, 720,
                                     SDL_LOGICAL_PRESENTATION_LETTERBOX);
    computePanelLayout();

    m_font = TTF_OpenFont("data/fonts/ShareTechMono-Regular.ttf", 16);
    if (!m_font)
        SDL_Log("SDL3Renderer: font (16pt) load failed: %s", SDL_GetError());

    m_fontLarge = TTF_OpenFont("data/fonts/ShareTechMono-Regular.ttf", 22);
    if (!m_fontLarge)
        SDL_Log("SDL3Renderer: fontLarge (22pt) load failed: %s", SDL_GetError());

    m_fontDamage = TTF_OpenFont("data/fonts/ShareTechMono-Regular.ttf", 20);
    if (!m_fontDamage)
        SDL_Log("SDL3Renderer: fontDamage (20pt) load failed: %s", SDL_GetError());

    SDL_SetRenderDrawColor(m_renderer, 11, 11, 18, 255);
    SDL_RenderClear(m_renderer);
    SDL_RenderPresent(m_renderer);
    m_lastFrameTicks = SDL_GetTicks();
}

SDL3Renderer::~SDL3Renderer()
{
    clearTextCache();
    clearDamageTextures();
    if (m_fontDamage)
    {
        TTF_CloseFont(m_fontDamage);
        m_fontDamage = nullptr;
    }
    if (m_fontLarge)
    {
        TTF_CloseFont(m_fontLarge);
        m_fontLarge = nullptr;
    }
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

void SDL3Renderer::computePanelLayout()
{
    constexpr float turnH = 72.f;
    constexpr float logH = 130.f;
    constexpr float hintH = 26.f;
    constexpr float mainH = 720.f - turnH - logH - hintH;
    constexpr float menuW = 537.f;
    constexpr float statW = 200.f;
    constexpr float rfW = 80.f;

    m_turnOrderPanel = {0.f, 0.f, 1280.f, turnH};
    m_playerPanel = {0.f, turnH, statW, mainH};
    m_centerPanel = {(1280.f - rfW) / 2.f, turnH, rfW, mainH};
    m_enemyPanel = {1280.f - statW, turnH, statW, mainH};
    m_actionMenu = {0.f, turnH + mainH, menuW, logH};
    m_logPanel = {menuW, turnH + mainH, 1280.f - menuW, logH};
    m_hintBar = {0.f, 720.f - hintH, 1280.f, hintH};
}

void SDL3Renderer::redrawAll()
{
    advanceAnimations();

    SDL_SetRenderDrawColor(m_renderer, 11, 11, 18, 255);
    SDL_RenderClear(m_renderer);

    // Draw order: background → battle area floor → panels → HUD → effects → vignette → overlays.
    drawBattleArea();
    drawTurnOrderStrip();
    renderBorder(m_turnOrderPanel, 55, 55, 75);

    if (m_cachedPlayerParty || m_cachedEnemyParty)
    {
        drawPlayerPanel();
        drawEnemyPanel();
        renderBorder(m_playerPanel, 45, 45, 65);
        renderBorder(m_enemyPanel, 45, 45, 65);
    }
    // Center panel is always drawn — shows skeleton before first RF data arrives.
    drawCenterPanel();
    renderBorder(m_centerPanel, 45, 45, 65);
    if (m_cachedActiveCharacter && m_cachedActiveParty)
    {
        drawActionMenuPanel();
        renderBorder(m_actionMenu, 40, 40, 58);
    }
    drawLogPanel();
    renderBorder(m_logPanel, 40, 40, 58);
    drawHintBarPanel();

    drawFractureLines();
    drawDamageNumbers();

    // Low-HP danger vignette — drawn after all panels so it is visible screen-wide.
    if (m_cachedPlayerParty)
    {
        float minHpFrac = 1.f;
        for (std::size_t i = 0; i < m_cachedPlayerParty->size(); ++i)
        {
            const Unit *u = m_cachedPlayerParty->getUnitAt(i);
            if (!u || !u->isAlive())
                continue;
            const float frac = u->getMaxHp() > 0
                                   ? static_cast<float>(u->getHp()) / static_cast<float>(u->getMaxHp())
                                   : 0.f;
            minHpFrac = std::min(minHpFrac, frac);
        }
        // Threshold: ramps from 0 at 30 % HP to 1 at 0 % HP.
        if (minHpFrac < 0.30f)
            drawLowHpVignette(1.f - minHpFrac / 0.30f);
    }

    if (SDL_GetTicks() < m_rfTriggerExpiry)
    {
        const SDL_Color tc = affinityColor(m_rfTriggerAffinity);
        fillRect({0.f, 0.f, 1280.f, 720.f}, tc.r, tc.g, tc.b, 50);
        renderTextEx(m_fontLarge, "RESONANCE TRIGGER",
                     1280.f * 0.3f, 720.f * 0.45f, tc.r, tc.g, tc.b);
    }

    SDL_RenderPresent(m_renderer);
}

void SDL3Renderer::drawBattleArea()
{
    // The space between the three stat panels serves as the battlefield floor.
    // A slightly lighter shade than the background separates it visually,
    // providing a stage for future illustrated backgrounds.
    const float areaX = m_playerPanel.x + m_playerPanel.w;
    const float areaW = m_enemyPanel.x - areaX;
    const float areaY = m_turnOrderPanel.y + m_turnOrderPanel.h;
    const float areaH = m_actionMenu.y - areaY;
    fillRect({areaX, areaY, areaW, areaH}, 15, 15, 24);

    // Very faint horizontal lines add subtle depth without requiring a texture asset.
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 7);
    for (float y = areaY + 20.f; y < areaY + areaH; y += 20.f)
        SDL_RenderLine(m_renderer, areaX, y, areaX + areaW, y);
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_NONE);
}

void SDL3Renderer::drawTurnOrderStrip()
{
    fillRect(m_turnOrderPanel, 26, 26, 42, 255);
    fillRect({m_turnOrderPanel.x, m_turnOrderPanel.y, m_turnOrderPanel.w, 1.f}, 55, 55, 80);

    if (m_cachedTurnOrder.empty() || !m_font)
        return;

    const float slotW = m_turnOrderPanel.w / static_cast<float>(m_cachedTurnOrder.size());
    const float badgeH = m_turnOrderPanel.h * 0.72f;
    const float badgeY = m_turnOrderPanel.y + (m_turnOrderPanel.h - badgeH) * 0.5f;

    // Pre-populate the seen set with units that have already acted this cycle.
    // This shifts the first divider to the true cycle boundary instead of the
    // first repeat that happens to appear in the projected strip.
    std::vector<bool> hasDividerBefore(m_cachedTurnOrder.size(), false);
    {
        std::vector<const Unit *> seenThisCycle(m_actedThisCycle.begin(),
                                                m_actedThisCycle.end());
        for (std::size_t i = 0; i < m_cachedTurnOrder.size(); ++i)
        {
            const Unit *u = m_cachedTurnOrder[i].unit;
            if (!u)
                continue;
            const bool alreadySeen =
                std::find(seenThisCycle.begin(), seenThisCycle.end(), u) != seenThisCycle.end();
            if (alreadySeen)
            {
                hasDividerBefore[i] = true;
                seenThisCycle.clear(); // subsequent cycles start with a clean slate
            }
            seenThisCycle.push_back(u);
        }
    }

    for (std::size_t i = 0; i < m_cachedTurnOrder.size(); ++i)
    {
        const TurnSlot &slot = m_cachedTurnOrder[i];
        if (!slot.unit)
            continue;

        const float badgeX = m_turnOrderPanel.x + static_cast<float>(i) * slotW + slotW * 0.05f + m_stripSlideOffset;
        const float badgeW = slotW * 0.90f;
        const SDL_FRect badge{badgeX, badgeY, badgeW, badgeH};
        const SDL_Color ac = affinityColor(slot.unit->getAffinity());
        const bool isActive = (i == 0);

        // Badge fill.
        if (slot.isPlayer)
            renderChamferedFillRect(badge, 4.f, ac.r, ac.g, ac.b);
        else
            renderChamferedFillRect(badge, 4.f,
                                    toUint8(ac.r * 2 / 5),
                                    toUint8(ac.g * 2 / 5),
                                    toUint8(ac.b * 2 / 5));

        // Border color: blue for allies, red for enemies — same regardless of active state.
        const Uint8 br = slot.isPlayer ? 90 : 200;
        const Uint8 bg = slot.isPlayer ? 175 : 55;
        const Uint8 bb = slot.isPlayer ? 215 : 55;

        // Active slot: glow passes outward at increasing expansion.
        if (isActive)
        {
            SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
            renderChamferedBorder({badgeX - 3.f, badgeY - 3.f, badgeW + 6.f, badgeH + 6.f}, 7.f, br, bg, bb, 20);
            renderChamferedBorder({badgeX - 2.f, badgeY - 2.f, badgeW + 4.f, badgeH + 4.f}, 6.f, br, bg, bb, 45);
            renderChamferedBorder({badgeX - 1.f, badgeY - 1.f, badgeW + 2.f, badgeH + 2.f}, 5.f, br, bg, bb, 90);
            SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_NONE);
        }

        // Sandwich: outer dark → colored → inner dark.
        renderChamferedBorder({badgeX - 1.f, badgeY - 1.f, badgeW + 2.f, badgeH + 2.f}, 5.f, 8, 8, 14, 200);
        renderChamferedBorder(badge, 4.f, br, bg, bb, isActive ? 240 : 180);
        renderChamferedBorder({badgeX + 1.f, badgeY + 1.f, badgeW - 2.f, badgeH - 2.f}, 3.f, 8, 8, 14, 160);

        // Initial letter — shifted right to clear the arrow on the active slot.
        const float textOffsetX = isActive ? badgeW * 0.42f : badgeW * 0.28f;
        renderText(slot.unit->getName().substr(0, 1),
                   badgeX + textOffsetX, badgeY + badgeH * 0.18f, 255, 255, 255);

        // Active slot: right-pointing arrow using SDL_RenderGeometry.
        if (isActive)
        {
            const float arrowCx = badgeX + 5.f;
            const float arrowCy = badgeY + badgeH * 0.5f;
            const float aw = 9.f, ah = 13.f;
            const SDL_FPoint z{0.f, 0.f};

            SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
            const SDL_FColor glow{1.f, 1.f, 1.f, 0.25f};
            const SDL_Vertex gv[3] = {
                {{arrowCx + aw + 3.f, arrowCy}, glow, z},
                {{arrowCx - 2.f, arrowCy - ah * 0.5f - 2.f}, glow, z},
                {{arrowCx - 2.f, arrowCy + ah * 0.5f + 2.f}, glow, z},
            };
            SDL_RenderGeometry(m_renderer, nullptr, gv, 3, nullptr, 0);

            const SDL_FColor white{1.f, 1.f, 1.f, 0.90f};
            const SDL_Vertex av[3] = {
                {{arrowCx + aw, arrowCy}, white, z},
                {{arrowCx, arrowCy - ah * 0.5f}, white, z},
                {{arrowCx, arrowCy + ah * 0.5f}, white, z},
            };
            SDL_RenderGeometry(m_renderer, nullptr, av, 3, nullptr, 0);
            SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_NONE);
        }

        // Cycle divider: filled 2px rect extending above and below the badge.
        if (i + 1 < m_cachedTurnOrder.size() && hasDividerBefore[i + 1])
        {
            const float divX = badgeX + badgeW + (slotW - badgeW) * 0.5f;
            if (divX > m_turnOrderPanel.x && divX < m_turnOrderPanel.x + m_turnOrderPanel.w)
            {
                constexpr float kOverhang = 7.f; // extends above/below the badge
                const float divY = badgeY - kOverhang;
                const float divH = badgeH + kOverhang * 2.f;

                SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
                // Glow halo behind divider.
                SDL_SetRenderDrawColor(m_renderer, 200, 210, 230, 40);
                SDL_FRect halo{divX - 3.f, divY, 6.f, divH};
                SDL_RenderFillRect(m_renderer, &halo);
                // Solid divider body.
                SDL_SetRenderDrawColor(m_renderer, 200, 210, 230, 160);
                SDL_FRect bar{divX - 1.f, divY, 2.f, divH};
                SDL_RenderFillRect(m_renderer, &bar);
                // Tick diamonds at top and bottom.
                SDL_FRect topTick{divX - 3.f, divY - 1.f, 6.f, 3.f};
                SDL_FRect botTick{divX - 3.f, divY + divH - 2.f, 6.f, 3.f};
                SDL_SetRenderDrawColor(m_renderer, 220, 225, 245, 200);
                SDL_RenderFillRect(m_renderer, &topTick);
                SDL_RenderFillRect(m_renderer, &botTick);
                SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_NONE);
            }
        }
    }
}

float SDL3Renderer::playerCardHeight(const Unit *u, bool isActive) const
{
    if (!u)
        return 0.f;
    if (!u->isAlive())
        return kCardTopPad + kPCNameH + 6.f + kPCBottomGap; // dead card

    // Portrait side: top pad + portrait + inner bottom + gap between cards.
    const float portraitSide = kCardTopPad + kPCPortraitH + kCardBotPad + kPCBottomGap;

    // Content side: accumulate all bar heights.
    float contentH = kCardTopPad + kPCNameH + kNameBarGap + kPCBarH + kPCGapH;
    if (u->getTotalShieldAmount() > 0)
        contentH += 4.f + kPCGapH; // shield bar + gap
    if (const auto *pc = dynamic_cast<const PlayableCharacter *>(u))
    {
        if (isActive)
            contentH += kPCThinH + kPCGapH + kPCThinH; // energy + gap + exposure
        else
            contentH += kPCThinH; // exposure only
        if (pc->isFractured())
            contentH += 14.f;
    }
    contentH += kCardBotPad + kPCBottomGap;

    return std::max(portraitSide, contentH);
}

float SDL3Renderer::enemyCardHeight(const Unit *u) const
{
    if (!u)
        return 0.f;

    // Portrait side.
    const float portraitSide = kCardTopPad + kEPortraitH + kCardBotPad + kEBottomGap;

    // Content side.
    float contentH = kCardTopPad + kENameH + kNameBarGap + kEBarH + kEGapH;
    if (u->getTotalShieldAmount() > 0)
        contentH += 4.f + kEGapH; // shield bar + gap
    contentH += kEThinH + 4.f;    // toughness bar + spacing below it
    if (!u->getIntentLabel().empty())
        contentH += kEIntentH;
    contentH += kCardBotPad + kEBottomGap;

    return std::max(portraitSide, contentH);
}

void SDL3Renderer::drawPlayerCard(const Unit *u, float &py, bool isActive, bool highlighted)
{
    if (!u)
        return;

    const float totalH = playerCardHeight(u, isActive);
    const float shakeX = computeShakeOffset(u);
    const SDL_Color ac = affinityColor(u->getAffinity());

    // Affinity accent strip on left edge — wider when highlighted.
    if (u->isAlive())
    {
        const float stripW = highlighted ? 10.f : (isActive ? 4.f : 2.f);
        fillRect({m_playerPanel.x + shakeX, py, stripW, totalH - kPCBottomGap},
                 ac.r, ac.g, ac.b);
    }

    if (!u->isAlive())
    {
        renderText("  " + u->getName() + " [KO]",
                   m_playerPanel.x + 8.f + shakeX, py + kCardTopPad, 70, 70, 70);
        py += totalH;
        return;
    }

    const float cardH = totalH - kPCBottomGap;
    const float cardX = m_playerPanel.x + 4.f + shakeX;
    const SDL_FRect cardRect{cardX, py, m_playerPanel.w - 8.f, cardH};

    // Drop shadow.
    {
        const SDL_FColor shadowFar{0.f, 0.f, 0.f, 0.18f};
        const SDL_FColor shadowNear{0.f, 0.f, 0.f, 0.30f};
        const SDL_FPoint z{0.f, 0.f};
        const float ox = 4.f, oy = 4.f;
        const SDL_Vertex sv[6] = {
            {{cardRect.x + ox, cardRect.y + oy}, shadowFar, z},
            {{cardRect.x + cardRect.w + ox, cardRect.y + oy}, shadowFar, z},
            {{cardRect.x + cardRect.w + ox, cardRect.y + cardRect.h + oy}, shadowNear, z},
            {{cardRect.x + ox, cardRect.y + cardRect.h + oy}, shadowNear, z},
            {{cardRect.x + ox, cardRect.y + oy}, shadowFar, z},
            {{cardRect.x + cardRect.w + ox, cardRect.y + cardRect.h + oy}, shadowNear, z},
        };
        SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
        SDL_RenderGeometry(m_renderer, nullptr, sv, 6, nullptr, 0);
        SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_NONE);
    }

    renderChamferedFillRect(cardRect, 3.f,
                            highlighted ? 35 : 24,
                            highlighted ? 40 : 24,
                            highlighted ? 52 : 36);
    renderChamferedBorder(cardRect, 3.f, 50, 50, 70, 180);
    fillRect({cardRect.x + 3.f, cardRect.y, cardRect.w - 6.f, 1.f}, 60, 60, 85, 160);

    // Portrait — right corners chamfered (face toward card content).
    const float portX = m_playerPanel.x + kPCPortraitPad + shakeX;
    const float portY = py + kCardTopPad;
    const SDL_FRect portrait{portX, portY, kPCPortraitW, kPCPortraitH};

    fillRect(portrait, 14, 14, 20);
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(m_renderer, ac.r, ac.g, ac.b, 55);
    SDL_RenderFillRect(m_renderer, &portrait);
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_NONE);

    // Asymmetric border: chamfer only right corners (toward card center).
    {
        constexpr float cut = 4.f;
        const float x = portX, y = portY, w = kPCPortraitW, h = kPCPortraitH;
        const SDL_FPoint pts[7] = {
            {x, y},               // top-left  (square)
            {x + w - cut, y},     // top edge before chamfer
            {x + w, y + cut},     // top-right chamfer
            {x + w, y + h - cut}, // right edge
            {x + w - cut, y + h}, // bottom-right chamfer
            {x, y + h},           // bottom-left (square)
            {x, y},               // close
        };
        SDL_SetRenderDrawColor(m_renderer, ac.r, ac.g, ac.b, 200);
        SDL_RenderLines(m_renderer, pts, 7);
    }

    renderTextEx(m_fontLarge, u->getName().substr(0, 1),
                 portX + kPCPortraitW * 0.28f, portY + kPCPortraitH * 0.15f,
                 ac.r, ac.g, ac.b);

    // Name and bars — right of portrait, with name-bar gap.
    const float barX = m_playerPanel.x + kPCPortraitPad + kPCPortraitW + 4.f + shakeX;
    const float barW = m_playerPanel.w - (kPCPortraitPad + kPCPortraitW + 4.f) - 8.f;
    float cy = py + kCardTopPad;

    renderText(u->getName(), barX, cy, ac.r, ac.g, ac.b);
    cy += kPCNameH + kNameBarGap;

    const auto hpIt = m_hpBars.find(u);
    const float hpFrac = (hpIt != m_hpBars.end())
                             ? hpIt->second.displayed
                             : (u->getMaxHp() > 0
                                    ? static_cast<float>(u->getHp()) / static_cast<float>(u->getMaxHp())
                                    : 0.f);
    renderBar({barX, cy, barW, kPCBarH}, hpFrac, 55, 185, 65, 55, 22, 22);
    cy += kPCBarH + kPCGapH;

    const int shieldAmt = u->getTotalShieldAmount();
    if (shieldAmt > 0)
    {
        const float sFrac = std::min(1.f, static_cast<float>(shieldAmt) / static_cast<float>(u->getMaxHp()));
        renderBar({barX, cy, barW, 4.f}, sFrac, 100, 200, 255, 30, 40, 50);
        cy += 4.f + kPCGapH;
    }

    if (const auto *pc = dynamic_cast<const PlayableCharacter *>(u))
        drawPlayerCardBars(pc, barX, barW, cy, isActive);

    // Selection arrow drawn LAST (on top of portrait) — centered vertically on accent strip.
    if (highlighted)
    {
        const float arrowCx = m_playerPanel.x + shakeX + 3.f;
        const float arrowCy = py + (totalH - kPCBottomGap) * 0.5f;
        const SDL_FPoint z{0.f, 0.f};

        SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
        // Glow pass — slightly larger, low alpha.
        const SDL_FColor glow{1.f, 1.f, 1.f, 0.25f};
        const SDL_Vertex gv[3] = {
            {{arrowCx + 8.f, arrowCy}, glow, z},
            {{arrowCx - 1.f, arrowCy - 6.5f}, glow, z},
            {{arrowCx - 1.f, arrowCy + 6.5f}, glow, z},
        };
        SDL_RenderGeometry(m_renderer, nullptr, gv, 3, nullptr, 0);
        // Main arrow: white so it reads against any affinity color.
        const SDL_FColor white{1.f, 1.f, 1.f, 0.90f};
        const SDL_Vertex av[3] = {
            {{arrowCx + 6.f, arrowCy}, white, z},
            {{arrowCx, arrowCy - 5.5f}, white, z},
            {{arrowCx, arrowCy + 5.5f}, white, z},
        };
        SDL_RenderGeometry(m_renderer, nullptr, av, 3, nullptr, 0);
        SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_NONE);
    }

    py += totalH;
}

void SDL3Renderer::drawPlayerCardBars(const PlayableCharacter *pc,
                                      float barX, float barW, float &py, bool isActive)
{
    const float expFrac = static_cast<float>(pc->getExposure()) / 100.f;
    if (isActive)
    {
        const float enFrac = static_cast<float>(pc->getEnergy()) / 100.f;
        renderBar({barX, py, barW, kPCThinH}, enFrac, 235, 200, 40, 50, 40, 5);
        py += kPCThinH + kPCGapH;
        renderBar({barX, py, barW, kPCThinH}, expFrac, 220, 75, 0, 35, 18, 0);
        py += kPCThinH;
        if (pc->isFractured())
        {
            renderText("[FRAC]", barX, py, 200, 40, 40);
            py += 14.f;
        }
    }
    else
    {
        renderBar({barX, py, barW, kPCThinH}, expFrac, 220, 75, 0, 35, 18, 0);
        py += kPCThinH;
    }
}

void SDL3Renderer::drawEnemyCard(const Unit *u, float &ey, bool highlighted)
{
    if (!u)
        return;

    const float totalH = enemyCardHeight(u);
    const float shakeX = computeShakeOffset(u);
    const SDL_Color ac = affinityColor(u->getAffinity());

    // Affinity accent strip on right edge.
    fillRect({m_enemyPanel.x + m_enemyPanel.w - 2.f + shakeX, ey, 2.f, totalH - kEBottomGap},
             ac.r, ac.g, ac.b);

    const float cardH = totalH - kEBottomGap;
    const float cardX = m_enemyPanel.x + 2.f + shakeX;
    const SDL_FRect cardRect{cardX, ey, m_enemyPanel.w - 4.f, cardH};

    // Drop shadow.
    {
        const SDL_FColor shadowFar{0.f, 0.f, 0.f, 0.18f};
        const SDL_FColor shadowNear{0.f, 0.f, 0.f, 0.30f};
        const SDL_FPoint z{0.f, 0.f};
        const float ox = 4.f, oy = 4.f;
        const SDL_Vertex sv[6] = {
            {{cardRect.x + ox, cardRect.y + oy}, shadowFar, z},
            {{cardRect.x + cardRect.w + ox, cardRect.y + oy}, shadowFar, z},
            {{cardRect.x + cardRect.w + ox, cardRect.y + cardRect.h + oy}, shadowNear, z},
            {{cardRect.x + ox, cardRect.y + cardRect.h + oy}, shadowNear, z},
            {{cardRect.x + ox, cardRect.y + oy}, shadowFar, z},
            {{cardRect.x + cardRect.w + ox, cardRect.y + cardRect.h + oy}, shadowNear, z},
        };
        SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
        SDL_RenderGeometry(m_renderer, nullptr, sv, 6, nullptr, 0);
        SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_NONE);
    }

    renderChamferedFillRect(cardRect, 3.f,
                            highlighted ? 38 : 26,
                            highlighted ? 38 : 26,
                            highlighted ? 54 : 38);
    renderChamferedBorder(cardRect, 3.f, 50, 50, 70, 180);
    fillRect({cardRect.x + 3.f, cardRect.y, cardRect.w - 6.f, 1.f}, 60, 60, 85, 160);

    // Break flash.
    if (m_breakFlashUnit == u && SDL_GetTicks() < m_breakFlashExpiry)
    {
        const float t = 1.f - static_cast<float>(m_breakFlashExpiry - SDL_GetTicks()) / 500.f;
        const Uint8 flashAlpha = toUint8(static_cast<int>(80.f * (1.f - t)));
        SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(m_renderer, ac.r, ac.g, ac.b, flashAlpha);
        SDL_RenderFillRect(m_renderer, &cardRect);
        SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_NONE);
    }

    // Death flash — white fade-out over the card while the unit is dying.
    if (m_deathFlashUnit == u && SDL_GetTicks() < m_deathFlashExpiry)
    {
        const float t = 1.f - static_cast<float>(
                                  m_deathFlashExpiry - SDL_GetTicks()) /
                                  static_cast<float>(kDeathFlashDurationMs);
        const Uint8 flashAlpha =
            toUint8(static_cast<int>(200.f * (1.f - t)));
        SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, flashAlpha);
        SDL_RenderFillRect(m_renderer, &cardRect);
        SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_NONE);
    }

    // Portrait on far right, top-padded. Left corners chamfered (face toward bar content).
    const float portX = m_enemyPanel.x + m_enemyPanel.w - kEPortraitPadR - kEPortraitW + shakeX;
    const float portY = ey + kCardTopPad;
    const SDL_FRect ePortrait{portX, portY, kEPortraitW, kEPortraitH};

    fillRect(ePortrait, 14, 14, 20);
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(m_renderer, ac.r, ac.g, ac.b, 55);
    SDL_RenderFillRect(m_renderer, &ePortrait);
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_NONE);

    // Asymmetric border: chamfer only left corners (toward card bar area).
    {
        constexpr float cut = 4.f;
        const float x = portX, y = portY, w = kEPortraitW, h = kEPortraitH;
        const SDL_FPoint pts[7] = {
            {x + cut, y},     // top (after left chamfer)
            {x + w, y},       // top-right (square)
            {x + w, y + h},   // bottom-right (square)
            {x + cut, y + h}, // bottom (before left chamfer)
            {x, y + h - cut}, // bottom-left chamfer
            {x, y + cut},     // left edge
            {x + cut, y},     // close
        };
        SDL_SetRenderDrawColor(m_renderer, ac.r, ac.g, ac.b, 200);
        SDL_RenderLines(m_renderer, pts, 7);
    }

    renderTextEx(m_fontLarge, u->getName().substr(0, 1),
                 portX + kEPortraitW * 0.25f, portY + kEPortraitH * 0.12f,
                 ac.r, ac.g, ac.b);

    // Content cursor — local, does not alias ey.
    const float barX = m_enemyPanel.x + 6.f + shakeX;
    const float barW = portX - (m_enemyPanel.x + 6.f + shakeX) - 4.f;
    float cy = ey + kCardTopPad;

    renderText(u->getName(), barX, cy, ac.r, ac.g, ac.b);
    cy += kENameH + kNameBarGap;

    const auto hpIt = m_hpBars.find(u);
    const float hpFrac = (hpIt != m_hpBars.end())
                             ? hpIt->second.displayed
                             : (u->getMaxHp() > 0
                                    ? static_cast<float>(u->getHp()) / static_cast<float>(u->getMaxHp())
                                    : 0.f);
    renderBar({barX, cy, barW, kEBarH}, hpFrac, 200, 55, 55, 70, 25, 25);
    cy += kEBarH + kEGapH;

    const int shieldAmt = u->getTotalShieldAmount();
    if (shieldAmt > 0)
    {
        const float sFrac = std::min(1.f, static_cast<float>(shieldAmt) / static_cast<float>(u->getMaxHp()));
        renderBar({barX, cy, barW, 4.f}, sFrac, 100, 200, 255, 30, 40, 50);
        cy += 4.f + kEGapH;
    }

    if (const auto *e = dynamic_cast<const Enemy *>(u))
    {
        const auto tfIt = m_toughnessBars.find(u);
        const float tf = (tfIt != m_toughnessBars.end())
                             ? tfIt->second.displayed
                             : (e->getMaxToughness() > 0
                                    ? static_cast<float>(e->getToughness()) / static_cast<float>(e->getMaxToughness())
                                    : 0.f);
        const bool broken = e->isBroken();
        renderBar({barX, cy, barW, kEThinH}, tf,
                  broken ? 255 : 210, broken ? 80 : 210, broken ? 0 : 210,
                  50, 50, 50);
    }
    cy += kEThinH + 4.f;

    const std::string intent = u->getIntentLabel();
    if (!intent.empty())
        renderText(truncate(intent, 22), barX, cy, 190, 175, 100);

    // Selection arrow — drawn LAST so it overlays all card content.
    // Placed in the left margin of the card backing, clear of the portrait.
    if (highlighted)
    {
        const float arrowCx = cardRect.x + 7.f;
        const float arrowCy = ey + (totalH - kEBottomGap) * 0.5f;
        const SDL_FPoint z{0.f, 0.f};
        SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
        const SDL_FColor glow{1.f, 1.f, 1.f, 0.22f};
        const SDL_Vertex gv[3] = {
            {{arrowCx + 9.f, arrowCy}, glow, z},
            {{arrowCx - 1.f, arrowCy - 7.5f}, glow, z},
            {{arrowCx - 1.f, arrowCy + 7.5f}, glow, z},
        };
        SDL_RenderGeometry(m_renderer, nullptr, gv, 3, nullptr, 0);
        const SDL_FColor bright{1.f, 1.f, 1.f, 0.88f};
        const SDL_Vertex av[3] = {
            {{arrowCx + 7.f, arrowCy}, bright, z},
            {{arrowCx, arrowCy - 6.f}, bright, z},
            {{arrowCx, arrowCy + 6.f}, bright, z},
        };
        SDL_RenderGeometry(m_renderer, nullptr, av, 3, nullptr, 0);
        SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_NONE);
    }

    ey += totalH;
}

void SDL3Renderer::drawPlayerPanel()
{
    // Semi-transparent so the battle area background bleeds through slightly,
    // giving the panel depth against the darker battlefield.
    fillRect(m_playerPanel, 18, 18, 28, 210);
    fillRect({m_playerPanel.x, m_playerPanel.y, m_playerPanel.w, 1.f}, 38, 38, 58);
    if (!m_cachedPlayerParty || !m_font)
        return;

    float py = m_playerPanel.y + 8.f;
    int aliveIdx = 0;
    for (std::size_t i = 0; i < m_cachedPlayerParty->size(); ++i)
    {
        const Unit *u = m_cachedPlayerParty->getUnitAt(i);
        if (!u)
            continue;
        const bool highlighted = (!m_highlightingEnemies && m_highlightedTargetIndex == aliveIdx);
        const bool isActive = (m_cachedActiveCharacter && u == m_cachedActiveCharacter);
        drawPlayerCard(u, py, isActive, highlighted);
        if (u->isAlive())
            ++aliveIdx;
    }
}

void SDL3Renderer::drawEnemyPanel()
{
    fillRect(m_enemyPanel, 18, 18, 28, 210);
    fillRect({m_enemyPanel.x, m_enemyPanel.y, m_enemyPanel.w, 1.f}, 38, 38, 58);
    if (!m_cachedEnemyParty || !m_font)
        return;

    float ey = m_enemyPanel.y + 8.f;
    int aliveIdx = 0;
    for (std::size_t i = 0; i < m_cachedEnemyParty->size(); ++i)
    {
        const Unit *u = m_cachedEnemyParty->getUnitAt(i);
        if (!u || !u->isAlive())
        {
            if (m_deathFlashUnit == u &&
                SDL_GetTicks() < m_deathFlashExpiry)
                drawEnemyCard(u, ey, false);
            continue;
        }
        const bool highlighted = (m_highlightingEnemies && m_highlightedTargetIndex == aliveIdx);
        drawEnemyCard(u, ey, highlighted);
        ++aliveIdx;
    }
}

void SDL3Renderer::drawCenterPanel()
{
    fillRect(m_centerPanel, 13, 13, 22, 255);

    // Draw the panel skeleton even before renderResonanceField is first called,
    // so the spine is visible from battle start.
    renderText("RF", m_centerPanel.x + 4.f, m_centerPanel.y + 4.f, 160, 160, 220);

    // Affinity vote mini-bars — only if RF data available.
    float barY = m_centerPanel.y + 22.f;
    const float barW = m_centerPanel.w - 8.f;
    if (m_cachedResonanceField)
    {
        const std::array<Affinity, 5> affs{Affinity::Blaze, Affinity::Frost, Affinity::Tempest,
                                           Affinity::Terra, Affinity::Aether};
        for (Affinity a : affs)
        {
            const float votes = static_cast<float>(m_cachedResonanceField->getVotes(a));
            const SDL_Color ac = affinityColor(a);
            renderBar({m_centerPanel.x + 4.f, barY, barW, 5.f}, votes / 5.f,
                      ac.r, ac.g, ac.b, 28, 28, 38);
            barY += 9.f;
        }
    }
    else
    {
        // Placeholder affinity bars so the spine doesn't look empty.
        const std::array<Affinity, 5> affs{Affinity::Blaze, Affinity::Frost, Affinity::Tempest,
                                           Affinity::Terra, Affinity::Aether};
        for (Affinity a : affs)
        {
            const SDL_Color ac = affinityColor(a);
            renderBar({m_centerPanel.x + 4.f, barY, barW, 5.f}, 0.f,
                      ac.r, ac.g, ac.b, 28, 28, 38);
            barY += 9.f;
        }
    }

    const float gaugeH = m_centerPanel.h - (barY - m_centerPanel.y) - 8.f;
    if (gaugeH <= 4.f)
        return;

    const SDL_FRect gaugeBg{m_centerPanel.x + 4.f, barY + 4.f, barW, gaugeH};
    fillRect(gaugeBg, 28, 28, 38);
    renderBorder(gaugeBg, 40, 40, 55);

    if (!m_cachedResonanceField)
    {
        renderText("0%", m_centerPanel.x + 4.f, barY + 4.f + gaugeH * 0.05f, 100, 100, 140);
        return;
    }

    const Affinity leading = m_cachedResonanceField->getLeadingAffinity();
    SDL_Color lc = affinityColor(leading);
    lc.r = toUint8(static_cast<int>(lc.r * m_rfGaugePulse));
    lc.g = toUint8(static_cast<int>(lc.g * m_rfGaugePulse));
    lc.b = toUint8(static_cast<int>(lc.b * m_rfGaugePulse));
    if (m_rfGaugeDisplayed > 0.f)
    {
        const float fillH = gaugeH * m_rfGaugeDisplayed;
        fillRect({gaugeBg.x, gaugeBg.y + gaugeH - fillH, gaugeBg.w, fillH},
                 lc.r, lc.g, lc.b);
    }
    renderText(std::to_string(static_cast<int>(m_rfGaugeDisplayed * 100.f)) + "%",
               m_centerPanel.x + 4.f, barY + 4.f + gaugeH * 0.05f, 200, 200, 255);
}

void SDL3Renderer::drawActionMenuPanel()
{
    fillRect(m_actionMenu, 15, 15, 24, 255);
    fillRect({m_actionMenu.x, m_actionMenu.y, m_actionMenu.w, 1.f}, 40, 40, 62);
    if (!m_cachedActiveCharacter || !m_cachedActiveParty || !m_font)
        return;

    renderText(m_cachedActiveCharacter->getName(),
               m_actionMenu.x + 6.f, m_actionMenu.y + 4.f, 210, 210, 220);
    renderText("SP:" + std::to_string(m_cachedActiveParty->getSp()),
               m_actionMenu.x + 116.f, m_actionMenu.y + 4.f, 180, 210, 255);
    fillRect({m_actionMenu.x + 4.f, m_actionMenu.y + 20.f, m_actionMenu.w - 8.f, 1.f}, 50, 50, 70);

    static const std::array<std::string, 6> kKeys{"Q", "E", "1", "2", "R", "V"};
    float rowY = m_actionMenu.y + 26.f;
    constexpr float rowH = 22.f;
    std::size_t keyIdx = 0;

    for (const auto &action : m_cachedActiveCharacter->getAbilities())
    {
        if (!action || keyIdx >= kKeys.size())
            break;
        const ActionData &data = action->getActionData();
        if (data.category == ActionCategory::ArchSkill &&
            !m_cachedActiveCharacter->isArchSkillUnlocked())
            continue;
        const bool available = action->isAvailable(*m_cachedActiveCharacter, *m_cachedActiveParty);
        const std::string row = "[" + kKeys[keyIdx] + "] " + action->label();
        if (available)
        {
            const SDL_Color ac = affinityColor(m_cachedActiveCharacter->getAffinity());
            renderText(row, m_actionMenu.x + 6.f, rowY, ac.r, ac.g, ac.b);
        }
        else
        {
            renderText(row, m_actionMenu.x + 6.f, rowY, 70, 70, 70);
        }
        rowY += rowH;
        ++keyIdx;
    }
}

void SDL3Renderer::drawLogPanel()
{
    fillRect(m_logPanel, 12, 12, 18, 255);
    fillRect({m_logPanel.x, m_logPanel.y, m_logPanel.w, 1.f}, 35, 35, 55);
    if (!m_font || m_log.empty())
        return;

    constexpr float lineH = 18.f;
    const int visibleLines = static_cast<int>((m_logPanel.h - 4.f) / lineH);
    const int total = static_cast<int>(m_log.size());
    const int start = std::max(0, total - visibleLines - m_logScrollOffset);
    float ly = m_logPanel.y + 4.f;

    for (int i = start; i < total && ly + lineH <= m_logPanel.y + m_logPanel.h; ++i)
    {
        const int age = total - 1 - i;
        const Uint8 brightness = toUint8(std::max(70, 210 - age * 25));
        const LogEntry &entry = m_log[static_cast<std::size_t>(i)];
        renderText(entry.text, m_logPanel.x + 6.f, ly,
                   toUint8(entry.color.r * brightness / 255),
                   toUint8(entry.color.g * brightness / 255),
                   toUint8(entry.color.b * brightness / 255));
        ly += lineH;
    }
}

void SDL3Renderer::drawHintBarPanel()
{
    fillRect(m_hintBar, 12, 12, 18, 255);
    if (m_font && !m_cachedHint.empty())
        renderText(m_cachedHint, m_hintBar.x + 8.f, m_hintBar.y + 4.f, 120, 120, 140);
}

void SDL3Renderer::drawFractureLines()
{
    const Uint64 now = SDL_GetTicks();
    for (const auto &fl : m_fractureLines)
    {
        if (now >= fl.birthTime + fl.durationMs)
            continue;
        const float progress = static_cast<float>(now - fl.birthTime) / static_cast<float>(fl.durationMs);
        const float alpha = 255.f * (1.f - progress);
        SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(m_renderer, fl.color.r, fl.color.g, fl.color.b,
                               toUint8(static_cast<int>(alpha)));
        SDL_RenderLine(m_renderer,
                       fl.startX, fl.startY,
                       fl.startX + std::cos(fl.angle) * fl.length * progress,
                       fl.startY + std::sin(fl.angle) * fl.length * progress);
        SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_NONE);
    }
    std::erase_if(m_fractureLines, [now](const FractureLine &fl)
                  { return now >= fl.birthTime + fl.durationMs; });
}

void SDL3Renderer::drawDamageNumbers()
{
    if (!m_fontDamage)
        return;

    const Uint64 now = SDL_GetTicks();
    for (const auto &dn : m_damageNumbers)
    {
        if (now >= dn.birthTime + static_cast<Uint64>(dn.durationMs))
            continue;

        const float age = static_cast<float>(now - dn.birthTime);
        const float progress = age / dn.durationMs;
        const float alpha = 255.f * (1.f - progress);
        const float yOffset = age * 0.025f;

        SDL_Texture *tex = getOrCreateDamageTexture(dn.text, dn.color);
        if (!tex)
            continue;

        float tw{}, th{};
        SDL_GetTextureSize(tex, &tw, &th);
        const float drawX = dn.spawnX - tw * 0.5f;
        const float drawY = dn.spawnY - yOffset;

        // Shadow pass: offset by 2 px, color-modulated to black.
        SDL_SetTextureAlphaMod(tex, toUint8(static_cast<int>(alpha * 0.65f)));
        SDL_SetTextureColorMod(tex, 0, 0, 0);
        const SDL_FRect shadowDst{drawX + 2.f, drawY + 2.f, tw, th};
        SDL_RenderTexture(m_renderer, tex, nullptr, &shadowDst);

        // Main pass: restore color.
        SDL_SetTextureColorMod(tex, 255, 255, 255);
        SDL_SetTextureAlphaMod(tex, toUint8(static_cast<int>(alpha)));
        const SDL_FRect dst{drawX, drawY, tw, th};
        SDL_RenderTexture(m_renderer, tex, nullptr, &dst);

        // Reset mods so cached textures are not left in a modified state.
        SDL_SetTextureAlphaMod(tex, 255);
    }
    std::erase_if(m_damageNumbers, [now](const DamageNumber &dn)
                  { return now >= dn.birthTime + static_cast<Uint64>(dn.durationMs); });
}

void SDL3Renderer::drawLowHpVignette(float severity)
{
    // severity is clamped to [0,1] by the caller.
    // Six layers with asymmetric (wider horizontal, narrower vertical) insets
    // approximate an elliptical falloff using only axis-aligned rects.
    struct Layer
    {
        float ix;
        float iy;
        float alpha;
    };
    constexpr std::array<Layer, 6> kLayers{{
        {0.f, 0.f, 60.f},
        {14.f, 9.f, 42.f},
        {26.f, 17.f, 26.f},
        {36.f, 24.f, 14.f},
        {44.f, 30.f, 7.f},
        {50.f, 34.f, 3.f},
    }};

    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
    for (const auto &layer : kLayers)
    {
        const Uint8 a = toUint8(static_cast<int>(layer.alpha * severity));
        SDL_SetRenderDrawColor(m_renderer, 160, 15, 15, a);
        const float w = 1280.f - layer.ix * 2.f;
        const float h = 720.f - layer.iy * 2.f;
        // Four edge bands per layer.
        const SDL_FRect edges[4] = {
            {layer.ix, layer.iy, w, 20.f},            // top
            {layer.ix, layer.iy + h - 20.f, w, 20.f}, // bottom
            {layer.ix, layer.iy, 20.f, h},            // left
            {layer.ix + w - 20.f, layer.iy, 20.f, h}, // right
        };
        SDL_RenderFillRects(m_renderer, edges, 4);
    }
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_NONE);
}

void SDL3Renderer::fillRect(const SDL_FRect &rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    SDL_SetRenderDrawBlendMode(m_renderer, a < 255 ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(m_renderer, r, g, b, a);
    SDL_RenderFillRect(m_renderer, &rect);
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_NONE);
}

void SDL3Renderer::renderBorder(const SDL_FRect &rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    SDL_SetRenderDrawBlendMode(m_renderer, a < 255 ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(m_renderer, r, g, b, a);
    SDL_RenderRect(m_renderer, &rect);
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_NONE);
}

void SDL3Renderer::renderChamferedFillRect(const SDL_FRect &rect, float cut,
                                           Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    // Eight-vertex convex polygon (chamfered octagon) drawn via SDL_RenderGeometry.
    // Avoids the hairline seam that appears between the two overlapping rects at
    // certain sub-pixel positions, and produces a cleaner corner angle.
    const float x0 = rect.x, x1 = rect.x + cut;
    const float x2 = rect.x + rect.w - cut, x3 = rect.x + rect.w;
    const float y0 = rect.y, y1 = rect.y + cut;
    const float y2 = rect.y + rect.h - cut, y3 = rect.y + rect.h;

    const SDL_FColor fc{r / 255.f, g / 255.f, b / 255.f, a / 255.f};
    const SDL_FPoint z{0.f, 0.f};

    // 8 edge vertices; triangle fan from the center requires 8 triangles (24 vertices).
    const float cx = rect.x + rect.w * 0.5f;
    const float cy = rect.y + rect.h * 0.5f;
    const SDL_Vertex edge[8] = {
        {{x1, y0}, fc, z},
        {{x2, y0}, fc, z},
        {{x3, y1}, fc, z},
        {{x3, y2}, fc, z},
        {{x2, y3}, fc, z},
        {{x1, y3}, fc, z},
        {{x0, y2}, fc, z},
        {{x0, y1}, fc, z},
    };
    const SDL_Vertex center{{cx, cy}, fc, z};

    SDL_Vertex tris[24];
    for (int i = 0; i < 8; ++i)
    {
        tris[i * 3] = center;
        tris[i * 3 + 1] = edge[i];
        tris[i * 3 + 2] = edge[(i + 1) % 8];
    }

    if (a < 255)
        SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderGeometry(m_renderer, nullptr, tris, 24, nullptr, 0);
    if (a < 255)
        SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_NONE);
}

void SDL3Renderer::renderChamferedBorder(const SDL_FRect &rect, float cut,
                                         Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    // SDL_RenderLines traces the octagon outline matching renderChamferedFillRect's corners.
    const float x = rect.x, y = rect.y, w = rect.w, h = rect.h;
    const SDL_FPoint pts[9] = {
        {x + cut, y},
        {x + w - cut, y},
        {x + w, y + cut},
        {x + w, y + h - cut},
        {x + w - cut, y + h},
        {x + cut, y + h},
        {x, y + h - cut},
        {x, y + cut},
        {x + cut, y}, // close
    };
    SDL_SetRenderDrawBlendMode(m_renderer, a < 255 ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(m_renderer, r, g, b, a);
    SDL_RenderLines(m_renderer, pts, 9);
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_NONE);
}

void SDL3Renderer::renderBar(const SDL_FRect &rect, float fraction,
                             Uint8 fr, Uint8 fg, Uint8 fb,
                             Uint8 br, Uint8 bg, Uint8 bb)
{
    fillRect(rect, br, bg, bb);
    renderBorder(rect, 50, 50, 65, 160);
    if (fraction > 0.f)
    {
        const float fillW = rect.w * std::clamp(fraction, 0.f, 1.f);
        fillRect({rect.x, rect.y, fillW, rect.h}, fr, fg, fb);
        // Right-edge highlight gives the filled bar a slight sheen.
        if (fillW >= 2.f)
            fillRect({rect.x + fillW - 1.f, rect.y, 1.f, rect.h},
                     toUint8(static_cast<int>(fr) + 60),
                     toUint8(static_cast<int>(fg) + 60),
                     toUint8(static_cast<int>(fb) + 60));
    }
}

void SDL3Renderer::renderText(const std::string &text, float x, float y,
                              Uint8 r, Uint8 g, Uint8 b)
{
    if (!m_font || text.empty())
        return;

    // Cache key encodes color so different-colored uses of identical strings
    // get separate textures. '\0' separates the text from the color bytes
    // to prevent collisions between text ending in a digit and a color byte
    // with the same value.
    const std::string key = text + '\0' + static_cast<char>(r) + static_cast<char>(g) + static_cast<char>(b);
    SDL_Texture *tex = nullptr;
    if (const auto it = m_textCache.find(key); it != m_textCache.end())
    {
        tex = it->second;
    }
    else
    {
        const SDL_Color color{r, g, b, 255};
        SDL_Surface *surf = TTF_RenderText_Blended(m_font, text.c_str(), 0, color);
        if (!surf)
            return;
        tex = SDL_CreateTextureFromSurface(m_renderer, surf);
        SDL_DestroySurface(surf);
        if (!tex)
            return;
        m_textCache[key] = tex;
    }

    float tw{}, th{};
    SDL_GetTextureSize(tex, &tw, &th);
    const SDL_FRect dst{x, y, tw, th};
    SDL_RenderTexture(m_renderer, tex, nullptr, &dst);
}

void SDL3Renderer::renderTextEx(TTF_Font *font, const std::string &text,
                                float x, float y, Uint8 r, Uint8 g, Uint8 b)
{
    if (!font || text.empty())
        return;
    const SDL_Color color{r, g, b, 255};
    SDL_Surface *surf = TTF_RenderText_Blended(font, text.c_str(), 0, color);
    if (!surf)
        return;
    SDL_Texture *tex = SDL_CreateTextureFromSurface(m_renderer, surf);
    SDL_DestroySurface(surf);
    if (!tex)
        return;
    float tw{}, th{};
    SDL_GetTextureSize(tex, &tw, &th);
    const SDL_FRect dst{x, y, tw, th};
    SDL_RenderTexture(m_renderer, tex, nullptr, &dst);
    SDL_DestroyTexture(tex);
}

void SDL3Renderer::fillRects(const std::vector<SDL_FRect> &rects,
                             Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    SDL_SetRenderDrawBlendMode(m_renderer, a < 255 ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(m_renderer, r, g, b, a);
    SDL_RenderFillRects(m_renderer, rects.data(), static_cast<int>(rects.size()));
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_NONE);
}

SDL_Texture *SDL3Renderer::getOrCreateDamageTexture(const std::string &text,
                                                    const SDL_Color &color)
{
    // Color is encoded in the key to prevent a damage "12" and a heal "+12"
    // sharing the same cached texture if they happen to use the same string.
    const std::string key = text + '\0' + static_cast<char>(color.r) + static_cast<char>(color.g) + static_cast<char>(color.b);
    if (const auto it = m_damageTextCache.find(key); it != m_damageTextCache.end())
        return it->second;

    TTF_Font *font = m_fontDamage ? m_fontDamage : m_font;
    if (!font)
        return nullptr;
    SDL_Surface *surf = TTF_RenderText_Blended(font, text.c_str(), 0, color);
    if (!surf)
        return nullptr;
    SDL_Texture *tex = SDL_CreateTextureFromSurface(m_renderer, surf);
    SDL_DestroySurface(surf);
    if (!tex)
        return nullptr;
    m_damageTextCache[key] = tex;
    return tex;
}

void SDL3Renderer::clearDamageTextures()
{
    for (auto &[k, t] : m_damageTextCache)
        SDL_DestroyTexture(t);
    m_damageTextCache.clear();
}

void SDL3Renderer::clearTextCache()
{
    for (auto &[k, t] : m_textCache)
        SDL_DestroyTexture(t);
    m_textCache.clear();
}

std::pair<float, float> SDL3Renderer::findCardCenter(const std::string &unitName,
                                                     bool searchEnemySide,
                                                     float fallbackX,
                                                     float fallbackY) const
{
    const Party *party = searchEnemySide ? m_cachedEnemyParty : m_cachedPlayerParty;
    const SDL_FRect &panel = searchEnemySide ? m_enemyPanel : m_playerPanel;
    if (!party)
        return {fallbackX, fallbackY};

    float cardY = panel.y + 8.f;
    for (std::size_t i = 0; i < party->size(); ++i)
    {
        const Unit *u = party->getUnitAt(i);
        if (!u)
            continue;

        if (searchEnemySide)
        {
            if (!u->isAlive())
            {
                // Dead enemy: card no longer drawn, but damage was just dealt so
                // return the position it occupied before dying (current cardY).
                if (u->getName() == unitName)
                    return {panel.x + panel.w * 0.5f, cardY + 20.f};
                // Dead enemies are not rendered, so they don't advance cardY.
                continue;
            }
            if (u->getName() == unitName)
                return {panel.x + panel.w * 0.5f, cardY + 20.f};
            cardY += enemyCardHeight(u);
        }
        else
        {
            // Dead PCs are rendered as "[KO]" entries, so they DO advance cardY.
            if (!u->isAlive())
            {
                if (u->getName() == unitName)
                    return {panel.x + panel.w * 0.5f, cardY + 20.f};
                cardY += kCardTopPad + kPCNameH + 6.f + kPCBottomGap; // matches dead-card path
                continue;
            }
            if (u->getName() == unitName)
                return {panel.x + panel.w * 0.5f, cardY + 20.f};
            const bool isActive = (m_cachedActiveCharacter && u == m_cachedActiveCharacter);
            cardY += playerCardHeight(u, isActive);
        }
    }
    return {fallbackX, fallbackY};
}

float SDL3Renderer::computeShakeOffset(const Unit *u) const
{
    if (!u)
        return 0.f;
    const auto it = m_shakes.find(u);
    if (it == m_shakes.end())
        return 0.f;
    const auto &sk = it->second;
    const Uint64 now = SDL_GetTicks();
    if (now >= sk.startMs + sk.durationMs)
        return 0.f;
    const float t = static_cast<float>(now - sk.startMs) / static_cast<float>(sk.durationMs);
    const float decay = 1.f - t;
    // 6 oscillations over the shake duration; amplitude decays linearly.
    return sk.intensity * decay * std::sin(t * 3.14159f * 6.f);
}

void SDL3Renderer::advanceAnimations()
{
    const Uint64 now = SDL_GetTicks();
    const float dtMs = static_cast<float>(now - m_lastFrameTicks);
    m_lastFrameTicks = now;
    const float factor = 1.f - std::exp(-0.01f * dtMs);

    for (auto &[u, bar] : m_hpBars)
        bar.displayed += (bar.target - bar.displayed) * factor;
    for (auto &[u, bar] : m_toughnessBars)
        bar.displayed += (bar.target - bar.displayed) * factor;
    m_rfGaugeDisplayed += (m_rfGaugeTarget - m_rfGaugeDisplayed) * factor;
    m_stripSlideOffset += (0.f - m_stripSlideOffset) * factor;
    m_rfGaugePulse += (1.f - m_rfGaugePulse) * 0.1f;
}

bool SDL3Renderer::animationPending() const
{
    constexpr float eps = 0.001f;
    for (const auto &[u, bar] : m_hpBars)
        if (std::abs(bar.displayed - bar.target) > eps)
            return true;
    for (const auto &[u, bar] : m_toughnessBars)
        if (std::abs(bar.displayed - bar.target) > eps)
            return true;
    return std::abs(m_rfGaugeDisplayed - m_rfGaugeTarget) > eps ||
           std::abs(m_stripSlideOffset) > eps;
}

bool SDL3Renderer::hasActiveVisualEffects() const
{
    if (!m_damageNumbers.empty())
        return true;
    if (m_breakFlashUnit && SDL_GetTicks() < m_breakFlashExpiry)
        return true;
    return !m_fractureLines.empty();
}

void SDL3Renderer::runAnimationFrames(int durationMs)
{
    const Uint64 end = SDL_GetTicks() + static_cast<Uint64>(durationMs);
    while (SDL_GetTicks() < end && animationPending())
    {
        advanceAnimations();
        redrawAll();
        SDL_Delay(16);
    }
    for (auto &[u, bar] : m_hpBars)
        bar.displayed = bar.target;
    for (auto &[u, bar] : m_toughnessBars)
        bar.displayed = bar.target;
    m_rfGaugeDisplayed = m_rfGaugeTarget;
    m_stripSlideOffset = 0.f;
}

void SDL3Renderer::flushAnimations()
{
    while (hasActiveVisualEffects())
    {
        advanceAnimations();
        redrawAll();
        SDL_Delay(16);
    }
}

void SDL3Renderer::flushVisualEffects()
{
    flushAnimations();
}

void SDL3Renderer::renderTurnOrder(const std::vector<TurnSlot> &order)
{
    if (!m_cachedTurnOrder.empty() && !order.empty() &&
        m_cachedTurnOrder[0].unit != order[0].unit)
    {
        // The acting unit changed — update m_actedThisCycle.
        // If the new slot 0 unit is already in m_actedThisCycle, the previous
        // cycle just completed and a fresh cycle is beginning; reset the set.
        // Otherwise, record the unit that just acted.
        const bool cycleComplete =
            order[0].unit != nullptr &&
            std::find(m_actedThisCycle.begin(), m_actedThisCycle.end(), order[0].unit) != m_actedThisCycle.end();

        if (cycleComplete)
            m_actedThisCycle.clear();
        else if (m_cachedTurnOrder[0].unit != nullptr)
            m_actedThisCycle.push_back(m_cachedTurnOrder[0].unit);

        // Slide all badges (and dividers) in from the right by one slot width.
        const float slotW = m_turnOrderPanel.w / static_cast<float>(order.empty() ? 1 : order.size());
        m_stripSlideOffset = slotW;
    }
    m_cachedTurnOrder = order;
    redrawAll();
}

void SDL3Renderer::renderPartyStatus(const Party &player, const Party &enemy)
{
    m_cachedPlayerParty = &player;
    m_cachedEnemyParty = &enemy;

    bool changed = false;
    auto updateHp = [&](const Party &p)
    {
        for (std::size_t i = 0; i < p.size(); ++i)
        {
            const Unit *u = p.getUnitAt(i);
            if (!u)
                continue;
            const float frac = u->getMaxHp() > 0
                                   ? static_cast<float>(u->getHp()) / static_cast<float>(u->getMaxHp())
                                   : 0.f;
            auto &bar = m_hpBars[u];
            if (std::abs(bar.target - frac) > 0.001f)
            {
                bar.target = frac;
                changed = true;
            }
        }
    };
    auto updateToughness = [&](const Party &p)
    {
        for (std::size_t i = 0; i < p.size(); ++i)
        {
            const Unit *u = p.getUnitAt(i);
            if (!u)
                continue;
            const auto *e = dynamic_cast<const Enemy *>(u);
            if (!e)
                continue;
            const float frac = e->getMaxToughness() > 0
                                   ? static_cast<float>(e->getToughness()) / static_cast<float>(e->getMaxToughness())
                                   : 0.f;
            auto &bar = m_toughnessBars[u];
            if (std::abs(bar.target - frac) > 0.001f)
            {
                bar.target = frac;
                changed = true;
            }
        }
    };
    updateHp(player);
    updateHp(enemy);
    updateToughness(enemy);

    if (changed)
        runAnimationFrames(300);
    else
        redrawAll();
}

void SDL3Renderer::renderResonanceField(const ResonanceField &field)
{
    const int newGauge = field.getGauge();
    if (m_cachedRfGauge >= 100 && newGauge == 0)
    {
        m_rfTriggerAffinity = field.getLeadingAffinity();
        m_rfTriggerExpiry = SDL_GetTicks() + 700;
        m_rfGaugePulse = 1.8f;
    }
    m_cachedRfGauge = newGauge;
    m_rfGaugeTarget = static_cast<float>(newGauge) / 100.f;
    m_cachedResonanceField = &field;
    redrawAll();
}

void SDL3Renderer::renderActionMenu(const PlayableCharacter &c, const Party &p)
{
    m_cachedActiveCharacter = &c;
    m_cachedActiveParty = &p;
    redrawAll();
}

void SDL3Renderer::renderHintBar(const std::string &hint)
{
    m_cachedHint = hint;
    redrawAll();
}

void SDL3Renderer::addLogMessage(const std::string &msg, SDL_Color color)
{
    m_log.push_back({msg, color});
    if (static_cast<int>(m_log.size()) > kMaxLogLines)
        m_log.erase(m_log.begin());
    m_logScrollOffset = 0;
    m_lastLogTime = SDL_GetTicks();
}

void SDL3Renderer::renderMessage(const std::string &msg)
{
    addLogMessage(msg);
    redrawAll();
}

void SDL3Renderer::renderActionResult(const std::string &actorName,
                                      const ActionResult &result)
{
    std::string line = actorName + ": ";
    SDL_Color msgColor{220, 220, 220, 255};

    switch (result.type)
    {
    case ActionResult::Type::Damage:
        line += "dealt " + std::to_string(result.value) + " dmg";
        if (!result.targetName.empty())
            line += " to " + result.targetName;
        msgColor = affinityColor(result.actionAffinity);
        break;
    case ActionResult::Type::Heal:
        line += "healed " + std::to_string(result.value) + " HP";
        if (!result.targetName.empty())
            line += " on " + result.targetName;
        msgColor = {100, 255, 100, 255};
        break;
    case ActionResult::Type::Skip:
        line += result.flavorText.empty() ? "skipped" : result.flavorText;
        break;
    case ActionResult::Type::Charge:
        line += "charges energy";
        msgColor = {255, 255, 100, 255};
        break;
    }
    if (!result.flavorText.empty() && result.type != ActionResult::Type::Skip)
        line += " \xe2\x80\x94 " + result.flavorText;

    addLogMessage(line, msgColor);

    if (result.type == ActionResult::Type::Damage || result.type == ActionResult::Type::Heal)
    {
        // Trigger a card shake on the target when damage lands.
        if (result.type == ActionResult::Type::Damage && result.value > 0 && !result.targetName.empty())
        {
            auto triggerShake = [&](const Party *party)
            {
                if (!party)
                    return;
                for (std::size_t i = 0; i < party->size(); ++i)
                {
                    const Unit *u = party->getUnitAt(i);
                    if (u && u->getName() == result.targetName)
                    {
                        m_shakes[u] = ShakeState{6.f, SDL_GetTicks(), 350};
                        return;
                    }
                }
            };
            triggerShake(m_cachedEnemyParty);
            triggerShake(m_cachedPlayerParty);
        }

        DamageNumber dn;
        dn.text = (result.type == ActionResult::Type::Damage)
                      ? std::to_string(result.value)
                      : "+" + std::to_string(result.value);
        dn.birthTime = SDL_GetTicks();
        dn.durationMs = 1200.f;
        dn.color = msgColor;

        if (!result.targetName.empty())
        {
            // Search enemy panel first (most common damage target).
            auto [ex, ey] = findCardCenter(result.targetName, true, 960.f, 300.f);
            if (ex != 960.f || ey != 300.f)
            {
                dn.spawnX = ex;
                dn.spawnY = ey;
            }
            else
            {
                auto [px, py] = findCardCenter(result.targetName, false, 100.f, 300.f);
                dn.spawnX = px;
                dn.spawnY = py;
            }
        }
        else
        {
            dn.spawnX = 100.f;
            dn.spawnY = 300.f;
        }
        m_damageNumbers.push_back(dn);
    }

    // Sync bar targets with current unit HP so they animate alongside
    // the damage number rather than waiting for the next renderPartyStatus.
    if (m_cachedPlayerParty)
    {
        for (std::size_t i = 0; i < m_cachedPlayerParty->size(); ++i)
        {
            const Unit *u = m_cachedPlayerParty->getUnitAt(i);
            if (!u)
                continue;
            const float frac = u->getMaxHp() > 0
                                   ? static_cast<float>(u->getHp()) /
                                         static_cast<float>(u->getMaxHp())
                                   : 0.f;
            m_hpBars[u].target = frac;
        }
    }
    if (m_cachedEnemyParty)
    {
        for (std::size_t i = 0; i < m_cachedEnemyParty->size(); ++i)
        {
            const Unit *u = m_cachedEnemyParty->getUnitAt(i);
            if (!u)
                continue;
            const float frac = u->getMaxHp() > 0
                                   ? static_cast<float>(u->getHp()) /
                                         static_cast<float>(u->getMaxHp())
                                   : 0.f;
            m_hpBars[u].target = frac;
            const auto *e = dynamic_cast<const Enemy *>(u);
            if (e && e->getMaxToughness() > 0)
                m_toughnessBars[u].target =
                    static_cast<float>(e->getToughness()) /
                    static_cast<float>(e->getMaxToughness());
        }
    }
    // Arm death flash for any enemy whose HP just reached 0 so the card
    // stays visible during runAnimationFrames instead of vanishing immediately.
    const bool flashAvailable = !m_deathFlashUnit ||
                                SDL_GetTicks() >= m_deathFlashExpiry;
    if (m_cachedEnemyParty && flashAvailable)
    {
        for (std::size_t i = 0; i < m_cachedEnemyParty->size(); ++i)
        {
            const Unit *u = m_cachedEnemyParty->getUnitAt(i);
            if (!u || u->isAlive())
                continue;
            const auto it = m_hpBars.find(u);
            if (it != m_hpBars.end() && it->second.displayed > 0.01f)
            {
                m_deathFlashUnit = u;
                m_deathFlashExpiry =
                    SDL_GetTicks() +
                    static_cast<Uint64>(kDeathFlashDurationMs);
                break;
            }
        }
    }
    runAnimationFrames(300);
}

void SDL3Renderer::renderBreak(const std::string &enemyName)
{
    if (m_cachedEnemyParty)
    {
        for (std::size_t i = 0; i < m_cachedEnemyParty->size(); ++i)
        {
            const Unit *u = m_cachedEnemyParty->getUnitAt(i);
            if (u && u->getName() == enemyName)
            {
                m_breakFlashUnit = u;
                m_breakFlashExpiry = SDL_GetTicks() + 500;
                break;
            }
        }
    }
    addLogMessage("** " + enemyName + " BROKEN **", {255, 80, 80, 255});
    redrawAll();
}

void SDL3Renderer::renderStunned(const std::string &name)
{
    addLogMessage(name + " stunned. Turn skipped");
    redrawAll();
}

void SDL3Renderer::renderVictory(const std::string &name,
                                 std::optional<Drop> drop)
{
    addLogMessage(name + " defeated!");
    if (drop)
        addLogMessage("  Drop: " + drop->itemId);
    redrawAll();
}

void SDL3Renderer::renderDefeat(const std::string &name)
{
    addLogMessage(name + " has fallen.");
    redrawAll();
}

void SDL3Renderer::renderTargetList(const std::vector<std::string> &, bool isAlly)
{
    m_highlightedTargetIndex = 0;
    m_highlightingEnemies = !isAlly;
    redrawAll();
}

void SDL3Renderer::clearTargetHighlight()
{
    m_highlightedTargetIndex = -1;
    redrawAll();
}

void SDL3Renderer::updateTargetHighlight(int idx)
{
    m_highlightedTargetIndex = idx;
    redrawAll();
}

void SDL3Renderer::presentPause(int ms)
{
    SDL_Delay(static_cast<Uint32>(ms));
}

void SDL3Renderer::renderSelectionMenu(const std::string &title,
                                       const std::vector<std::string> &options,
                                       std::size_t selected)
{
    // Shared geometry constants.
    constexpr float kPanelW = 1090.f; // ~85 % of the 1280 logical width
    constexpr float kTitleH = 58.f;
    constexpr float kRowH = 38.f;
    constexpr float kRowGap = 5.f;
    constexpr float kRowStep = kRowH + kRowGap;
    constexpr float kListTopPad = 12.f;
    constexpr float kListBotPad = 16.f;
    constexpr std::size_t kMaxVisibleRows = 11;
    constexpr float kHintH = 26.f;
    constexpr float kPanelChamfer = 6.f;

    // When dungeon select is active, re-draw the split layout instead.
    if (!m_dungeonSelectInfos.empty() && title == m_dungeonSelectTitle)
    {
        drawDungeonSelectScreen(title, m_dungeonSelectInfos, selected);
        SDL_RenderPresent(m_renderer);
        return;
    }
    // Entering a different menu — clear the dungeon context.
    m_dungeonSelectInfos.clear();
    m_dungeonSelectTitle.clear();

    // Background — same base as the battle screen.
    SDL_SetRenderDrawColor(m_renderer, 11, 11, 18, 255);
    SDL_RenderClear(m_renderer);

    // Faint horizontal grid lines (matches drawBattleArea visual language).
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 5);
    for (float gy = 20.f; gy < 720.f; gy += 20.f)
        SDL_RenderLine(m_renderer, 0.f, gy, 1280.f, gy);
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_NONE);

    // Hint bar — identical to the battle screen hint bar.
    fillRect({0.f, 720.f - kHintH, 1280.f, kHintH}, 12, 12, 18);
    fillRect({0.f, 720.f - kHintH, 1280.f, 1.f}, 35, 35, 55);
    if (m_font)
        renderText("[Up/Down] Navigate   [Enter] Confirm   [1-9] Direct select   [Esc] Back",
                   60.f, 720.f - kHintH + 5.f, 100, 100, 120);

    if (!m_fontLarge || options.empty())
    {
        SDL_RenderPresent(m_renderer);
        return;
    }

    // Compute the visible window — keeps selected row always in view.
    const std::size_t totalOpts  = options.size();
    const std::size_t visRows    = std::min(totalOpts, kMaxVisibleRows);
    const std::size_t windowStart =
        (selected >= visRows)
            ? std::min(selected - visRows + 1, totalOpts - visRows)
            : 0;

    // Panel height derived from visible row count.
    const float listH   = kListTopPad + static_cast<float>(visRows) * kRowStep + kListBotPad;
    const float panelH  = kTitleH + listH;
    const float panelX  = (1280.f - kPanelW) * 0.5f;
    const float panelY  = std::max(16.f, (720.f - kHintH - panelH) * 0.5f);
    const SDL_FRect panelRect{panelX, panelY, kPanelW, panelH};

    // Drop shadow — same two-triangle quad as card shadows in the battle screen.
    {
        constexpr float ox = 6.f, oy = 6.f;
        const SDL_FColor shadowFar {0.f, 0.f, 0.f, 0.22f};
        const SDL_FColor shadowNear{0.f, 0.f, 0.f, 0.42f};
        const SDL_FPoint z{0.f, 0.f};
        const SDL_Vertex sv[6] = {
            {{panelX + ox,          panelY + oy},          shadowFar,  z},
            {{panelX + kPanelW + ox, panelY + oy},         shadowFar,  z},
            {{panelX + kPanelW + ox, panelY + panelH + oy},shadowNear, z},
            {{panelX + ox,          panelY + panelH + oy}, shadowNear, z},
            {{panelX + ox,          panelY + oy},          shadowFar,  z},
            {{panelX + kPanelW + ox, panelY + panelH + oy},shadowNear, z},
        };
        SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
        SDL_RenderGeometry(m_renderer, nullptr, sv, 6, nullptr, 0);
        SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_NONE);
    }

    // Panel backing.
    renderChamferedFillRect(panelRect, kPanelChamfer, 18, 18, 28);
    // Sandwich border (outer dark → colored → inner dark).
    renderChamferedBorder({panelX - 1.f, panelY - 1.f, kPanelW + 2.f, panelH + 2.f},
                          kPanelChamfer + 1.f, 8, 8, 14, 210);
    renderChamferedBorder(panelRect, kPanelChamfer, 55, 70, 110, 210);
    renderChamferedBorder({panelX + 1.f, panelY + 1.f, kPanelW - 2.f, panelH - 2.f},
                          kPanelChamfer - 1.f, 8, 8, 14, 160);

    // Title sub-region.
    const SDL_FRect titleRect{panelX, panelY, kPanelW, kTitleH};
    renderChamferedFillRect(titleRect, kPanelChamfer, 24, 26, 42);
    // 1 px separator below title.
    fillRect({panelX + 8.f, panelY + kTitleH - 1.f, kPanelW - 16.f, 1.f}, 55, 70, 110, 200);
    // Top highlight.
    fillRect({panelX + kPanelChamfer, panelY, kPanelW - kPanelChamfer * 2.f, 1.f}, 70, 90, 140, 180);
    // Title text — left-padded.
    renderTextEx(m_fontLarge, title,
                 panelX + 22.f, panelY + (kTitleH - 22.f) * 0.5f,
                 190, 210, 255);

    // Option rows.
    float ry = panelY + kTitleH + kListTopPad;
    for (std::size_t vi = 0; vi < visRows; ++vi)
    {
        const std::size_t i = windowStart + vi;
        const bool sel      = (i == selected);
        const SDL_FRect rowRect{panelX + 10.f, ry, kPanelW - 20.f, kRowH};

        if (sel)
        {
            renderChamferedFillRect(rowRect, 3.f, 35, 42, 62);
            // Glow border passes (same pattern as active turn badge).
            SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
            renderChamferedBorder({rowRect.x - 2.f, rowRect.y - 2.f,
                                   rowRect.w + 4.f, rowRect.h + 4.f}, 5.f, 90, 130, 200, 28);
            renderChamferedBorder({rowRect.x - 1.f, rowRect.y - 1.f,
                                   rowRect.w + 2.f, rowRect.h + 2.f}, 4.f, 90, 130, 200, 70);
            SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_NONE);
            renderChamferedBorder(rowRect, 3.f, 90, 130, 200, 200);
        }
        else
        {
            renderChamferedFillRect(rowRect, 3.f, 22, 22, 34);
            renderChamferedBorder(rowRect, 3.f, 40, 40, 58, 120);
        }

        // Right-pointing selector arrow on selected row.
        if (sel)
        {
            const float arrowCx = rowRect.x + 10.f;
            const float arrowCy = ry + kRowH * 0.5f;
            const SDL_FPoint z{0.f, 0.f};
            SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
            const SDL_FColor glow {1.f, 1.f, 1.f, 0.22f};
            const SDL_Vertex gv[3] = {
                {{arrowCx + 9.f, arrowCy},        glow, z},
                {{arrowCx - 1.f, arrowCy - 6.f},  glow, z},
                {{arrowCx - 1.f, arrowCy + 6.f},  glow, z},
            };
            SDL_RenderGeometry(m_renderer, nullptr, gv, 3, nullptr, 0);
            const SDL_FColor white{1.f, 1.f, 1.f, 0.90f};
            const SDL_Vertex av[3] = {
                {{arrowCx + 7.f, arrowCy},        white, z},
                {{arrowCx,       arrowCy - 5.f},  white, z},
                {{arrowCx,       arrowCy + 5.f},  white, z},
            };
            SDL_RenderGeometry(m_renderer, nullptr, av, 3, nullptr, 0);
            SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_NONE);
        }

        // Number label.
        const float numX = rowRect.x + 22.f;
        const float textY = ry + (kRowH - 16.f) * 0.5f; // centers 16pt text in kRowH
        if (m_font)
            renderText(std::to_string(i + 1) + ".",
                       numX, textY,
                       sel ? 140 : 70, sel ? 160 : 78, sel ? 200 : 95);

        // Option text.
        const float optX = numX + 26.f;
        if (sel)
            renderTextEx(m_fontLarge, truncate(options[i], 90),
                         optX, ry + (kRowH - 22.f) * 0.5f,
                         220, 230, 255);
        else if (m_font)
            renderText(truncate(options[i], 90),
                       optX, textY, 160, 165, 185);

        ry += kRowStep;
    }

    // Scroll indicator — up triangle when window is scrolled down.
    if (windowStart > 0)
    {
        const float ix  = panelX + kPanelW * 0.5f;
        const float iy  = panelY + kTitleH + 3.f;
        const SDL_FPoint z{0.f, 0.f};
        const SDL_FColor ind{0.7f, 0.8f, 1.f, 0.70f};
        const SDL_Vertex tv[3] = {
            {{ix,        iy},       ind, z},
            {{ix - 7.f,  iy + 8.f}, ind, z},
            {{ix + 7.f,  iy + 8.f}, ind, z},
        };
        SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
        SDL_RenderGeometry(m_renderer, nullptr, tv, 3, nullptr, 0);
        SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_NONE);
    }

    // Scroll indicator — down triangle when more rows exist below the window.
    if (windowStart + visRows < totalOpts)
    {
        const float ix  = panelX + kPanelW * 0.5f;
        const float iy  = panelY + panelH - 10.f;
        const SDL_FPoint z{0.f, 0.f};
        const SDL_FColor ind{0.7f, 0.8f, 1.f, 0.70f};
        const SDL_Vertex tv[3] = {
            {{ix,        iy},       ind, z},
            {{ix - 7.f,  iy - 8.f}, ind, z},
            {{ix + 7.f,  iy - 8.f}, ind, z},
        };
        SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
        SDL_RenderGeometry(m_renderer, nullptr, tv, 3, nullptr, 0);
        SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_NONE);
    }

    SDL_RenderPresent(m_renderer);
}

void SDL3Renderer::renderDungeonSelect(const std::string &title,
                                       const std::vector<DungeonSelectInfo> &dungeons,
                                       std::size_t selected)
{
    m_dungeonSelectTitle = title;
    m_dungeonSelectInfos = dungeons;
    drawDungeonSelectScreen(title, dungeons, selected);
    SDL_RenderPresent(m_renderer);
}

void SDL3Renderer::drawDungeonSelectScreen(const std::string &title,
                                           const std::vector<DungeonSelectInfo> &dungeons,
                                           std::size_t selected)
{
    constexpr float kHintH = 26.f;
    constexpr float kMargin = 16.f;
    constexpr float kGap = 12.f;
    constexpr float kListW = 660.f;                                    // left panel width
    constexpr float kDetailW = 1280.f - kListW - kGap - kMargin * 2.f; // ~560 px
    constexpr float kPanelY = 20.f;
    const float kPanelH = 720.f - kHintH - kPanelY * 2.f;
    constexpr float kListX = kMargin;
    const float kDetailX = kListX + kListW + kGap;
    constexpr float kTitleH = 56.f;
    constexpr float kRowH = 38.f;
    constexpr float kRowGap = 5.f;
    constexpr float kRowStep = kRowH + kRowGap;
    constexpr float kListPad = 12.f;
    constexpr float kChamfer = 6.f;
    const std::size_t totalOpts = dungeons.size() + 1; // +1 for Back row
    constexpr std::size_t kMaxVis = 11;
    const std::size_t visRows = std::min(totalOpts, kMaxVis);

    // Background.
    SDL_SetRenderDrawColor(m_renderer, 11, 11, 18, 255);
    SDL_RenderClear(m_renderer);
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 5);
    for (float gy = 20.f; gy < 720.f; gy += 20.f)
        SDL_RenderLine(m_renderer, 0.f, gy, 1280.f, gy);
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_NONE);

    // Hint bar.
    fillRect({0.f, 720.f - kHintH, 1280.f, kHintH}, 12, 12, 18);
    fillRect({0.f, 720.f - kHintH, 1280.f, 1.f}, 35, 35, 55);
    if (m_font)
        renderText("[Up/Down] Navigate   [Enter] Confirm   [Esc] Back",
                   kListX + 4.f, 720.f - kHintH + 5.f, 100, 100, 120);

    // Scroll window.
    const std::size_t windowStart =
        (selected >= visRows) ? std::min(selected - visRows + 1, totalOpts - visRows) : 0;

    // Left list panel.
    const SDL_FRect listPanel{kListX, kPanelY, kListW, kPanelH};
    {
        const SDL_FColor sf{0.f, 0.f, 0.f, 0.28f};
        const SDL_FColor sn{0.f, 0.f, 0.f, 0.45f};
        const SDL_FPoint z{0.f, 0.f};
        const float ox = 5.f, oy = 5.f;
        const SDL_Vertex sv[6] = {
            {{kListX + ox, kPanelY + oy}, sf, z},
            {{kListX + kListW + ox, kPanelY + oy}, sf, z},
            {{kListX + kListW + ox, kPanelY + kPanelH + oy}, sn, z},
            {{kListX + ox, kPanelY + kPanelH + oy}, sn, z},
            {{kListX + ox, kPanelY + oy}, sf, z},
            {{kListX + kListW + ox, kPanelY + kPanelH + oy}, sn, z},
        };
        SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
        SDL_RenderGeometry(m_renderer, nullptr, sv, 6, nullptr, 0);
        SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_NONE);
    }
    renderChamferedFillRect(listPanel, kChamfer, 18, 18, 28);
    renderChamferedBorder({kListX - 1.f, kPanelY - 1.f, kListW + 2.f, kPanelH + 2.f}, kChamfer + 1.f, 8, 8, 14, 210);
    renderChamferedBorder(listPanel, kChamfer, 55, 70, 110, 210);
    renderChamferedBorder({kListX + 1.f, kPanelY + 1.f, kListW - 2.f, kPanelH - 2.f}, kChamfer - 1.f, 8, 8, 14, 160);

    // List title area.
    const SDL_FRect titleRect{kListX, kPanelY, kListW, kTitleH};
    renderChamferedFillRect(titleRect, kChamfer, 24, 26, 42);
    fillRect({kListX + 8.f, kPanelY + kTitleH - 1.f, kListW - 16.f, 1.f}, 55, 70, 110, 200);
    fillRect({kListX + kChamfer, kPanelY, kListW - kChamfer * 2.f, 1.f}, 70, 90, 140, 180);
    if (m_fontLarge)
        renderTextEx(m_fontLarge, title,
                     kListX + 22.f, kPanelY + (kTitleH - 22.f) * 0.5f,
                     190, 210, 255);

    // Dungeon rows.
    float ry = kPanelY + kTitleH + kListPad;
    for (std::size_t vi = 0; vi < visRows; ++vi)
    {
        const std::size_t i = windowStart + vi;
        const bool sel = (i == selected);
        const bool isBack = (i == dungeons.size());
        const SDL_FRect rowRect{kListX + 10.f, ry, kListW - 20.f, kRowH};

        if (sel)
        {
            renderChamferedFillRect(rowRect, 3.f, 35, 42, 62);
            SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
            renderChamferedBorder({rowRect.x - 2.f, rowRect.y - 2.f, rowRect.w + 4.f, rowRect.h + 4.f},
                                  5.f, 90, 130, 200, 28);
            renderChamferedBorder({rowRect.x - 1.f, rowRect.y - 1.f, rowRect.w + 2.f, rowRect.h + 2.f},
                                  4.f, 90, 130, 200, 70);
            SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_NONE);
            renderChamferedBorder(rowRect, 3.f, 90, 130, 200, 200);
        }
        else
        {
            renderChamferedFillRect(rowRect, 3.f, 22, 22, 34);
            renderChamferedBorder(rowRect, 3.f, 40, 40, 58, 120);
        }

        // Selection arrow.
        if (sel)
        {
            const float ax = rowRect.x + 10.f;
            const float ay = ry + kRowH * 0.5f;
            const SDL_FPoint z{0.f, 0.f};
            SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
            const SDL_FColor glow{1.f, 1.f, 1.f, 0.22f};
            const SDL_Vertex gv[3] = {
                {{ax + 9.f, ay}, glow, z},
                {{ax - 1.f, ay - 6.f}, glow, z},
                {{ax - 1.f, ay + 6.f}, glow, z},
            };
            SDL_RenderGeometry(m_renderer, nullptr, gv, 3, nullptr, 0);
            const SDL_FColor white{1.f, 1.f, 1.f, 0.90f};
            const SDL_Vertex av[3] = {
                {{ax + 7.f, ay}, white, z},
                {{ax, ay - 5.f}, white, z},
                {{ax, ay + 5.f}, white, z},
            };
            SDL_RenderGeometry(m_renderer, nullptr, av, 3, nullptr, 0);
            SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_NONE);
        }

        const float textY = ry + (kRowH - 16.f) * 0.5f;
        const float numX = rowRect.x + 22.f;
        const float optX = numX + 26.f;

        if (isBack)
        {
            if (m_font)
                renderText(std::to_string(i + 1) + ".", numX, textY,
                           sel ? 140 : 60, sel ? 160 : 68, sel ? 200 : 85);
            if (sel && m_fontLarge)
                renderTextEx(m_fontLarge, "<< Back", optX,
                             ry + (kRowH - 22.f) * 0.5f, 180, 180, 200);
            else if (m_font)
                renderText("<< Back", optX, textY, 120, 120, 140);
        }
        else
        {
            const auto &d = dungeons[i];
            if (m_font)
                renderText(std::to_string(i + 1) + ".", numX, textY,
                           sel ? 140 : 70, sel ? 160 : 78, sel ? 200 : 95);

            const std::string label = d.name + (d.cleared ? "  [CLEARED]" : "");
            if (sel && m_fontLarge)
                renderTextEx(m_fontLarge, truncate(label, 52),
                             optX, ry + (kRowH - 22.f) * 0.5f, 220, 230, 255);
            else if (m_font)
                renderText(truncate(label, 52), optX, textY, 160, 165, 185);
        }
        ry += kRowStep;
    }

    // Scroll indicators.
    if (windowStart > 0)
    {
        const float ix = kListX + kListW * 0.5f;
        const float iy = kPanelY + kTitleH + 3.f;
        const SDL_FColor ind{0.7f, 0.8f, 1.f, 0.70f};
        const SDL_FPoint z{0.f, 0.f};
        const SDL_Vertex tv[3]{
            {{ix, iy}, ind, z},
            {{ix - 7.f, iy + 8.f}, ind, z},
            {{ix + 7.f, iy + 8.f}, ind, z},
        };
        SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
        SDL_RenderGeometry(m_renderer, nullptr, tv, 3, nullptr, 0);
        SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_NONE);
    }
    if (windowStart + visRows < totalOpts)
    {
        const float ix = kListX + kListW * 0.5f;
        const float iy = kPanelY + kPanelH - 10.f;
        const SDL_FColor ind{0.7f, 0.8f, 1.f, 0.70f};
        const SDL_FPoint z{0.f, 0.f};
        const SDL_Vertex tv[3]{
            {{ix, iy}, ind, z},
            {{ix - 7.f, iy - 8.f}, ind, z},
            {{ix + 7.f, iy - 8.f}, ind, z},
        };
        SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
        SDL_RenderGeometry(m_renderer, nullptr, tv, 3, nullptr, 0);
        SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_NONE);
    }

    // Right detail panel.
    const SDL_FRect detailPanel{kDetailX, kPanelY, kDetailW, kPanelH};
    if (selected < dungeons.size())
        drawDungeonDetailPanel(detailPanel, dungeons[selected]);
    else
    {
        // "Back" selected — draw a neutral detail panel.
        renderChamferedFillRect(detailPanel, kChamfer, 18, 18, 28);
        renderChamferedBorder(detailPanel, kChamfer, 40, 40, 58, 160);
        if (m_font)
            renderText("Return to mode select.", kDetailX + 20.f,
                       kPanelY + kPanelH * 0.5f, 100, 100, 120);
    }
}

void SDL3Renderer::drawDungeonDetailPanel(const SDL_FRect &panel,
                                          const DungeonSelectInfo &info)
{
    constexpr float kChamfer = 6.f;
    constexpr float kPad = 20.f;

    // Drop shadow.
    {
        const SDL_FColor sf{0.f, 0.f, 0.f, 0.25f};
        const SDL_FColor sn{0.f, 0.f, 0.f, 0.42f};
        const SDL_FPoint z{0.f, 0.f};
        const float ox = 5.f, oy = 5.f;
        const SDL_Vertex sv[6] = {
            {{panel.x + ox, panel.y + oy}, sf, z},
            {{panel.x + panel.w + ox, panel.y + oy}, sf, z},
            {{panel.x + panel.w + ox, panel.y + panel.h + oy}, sn, z},
            {{panel.x + ox, panel.y + panel.h + oy}, sn, z},
            {{panel.x + ox, panel.y + oy}, sf, z},
            {{panel.x + panel.w + ox, panel.y + panel.h + oy}, sn, z},
        };
        SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
        SDL_RenderGeometry(m_renderer, nullptr, sv, 6, nullptr, 0);
        SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_NONE);
    }

    renderChamferedFillRect(panel, kChamfer, 20, 20, 32);
    renderChamferedBorder({panel.x - 1.f, panel.y - 1.f, panel.w + 2.f, panel.h + 2.f},
                          kChamfer + 1.f, 8, 8, 14, 200);
    renderChamferedBorder(panel, kChamfer, 55, 70, 110, 200);
    renderChamferedBorder({panel.x + 1.f, panel.y + 1.f, panel.w - 2.f, panel.h - 2.f},
                          kChamfer - 1.f, 8, 8, 14, 150);

    // Top accent line.
    fillRect({panel.x + kChamfer, panel.y, panel.w - kChamfer * 2.f, 1.f},
             70, 90, 140, 180);

    float cy = panel.y + kPad;

    // Dungeon name.
    if (m_fontLarge)
    {
        renderTextEx(m_fontLarge, info.name,
                     panel.x + kPad, cy, 210, 225, 255);
        cy += 28.f;
    }

    // Difficulty + Cleared badges.
    {
        const bool hard = (info.difficultyLabel == "Hard");
        const bool nightmare = (info.difficultyLabel == "Nightmare");
        const Uint8 dr = hard ? 220 : (nightmare ? 255 : 100);
        const Uint8 dg = hard ? 160 : (nightmare ? 80 : 180);
        const Uint8 db = hard ? 40 : (nightmare ? 40 : 100);
        const SDL_FRect diffBadge{panel.x + kPad, cy, 80.f, 18.f};
        renderChamferedFillRect(diffBadge, 3.f, toUint8(dr / 3), toUint8(dg / 3), toUint8(db / 3));
        renderChamferedBorder(diffBadge, 3.f, dr, dg, db, 200);
        if (m_font)
            renderText(info.difficultyLabel, diffBadge.x + 4.f, diffBadge.y + 1.f,
                       dr, dg, db);

        if (info.cleared)
        {
            const SDL_FRect clrBadge{panel.x + kPad + 88.f, cy, 70.f, 18.f};
            renderChamferedFillRect(clrBadge, 3.f, 20, 55, 25);
            renderChamferedBorder(clrBadge, 3.f, 80, 200, 100, 200);
            if (m_font)
                renderText("CLEARED", clrBadge.x + 4.f, clrBadge.y + 1.f,
                           80, 200, 100);
        }
        cy += 26.f;
    }

    // Horizontal separator.
    fillRect({panel.x + kPad, cy, panel.w - kPad * 2.f, 1.f}, 50, 55, 75, 180);
    cy += 12.f;

    // Stats row.
    if (m_font)
    {
        const std::string stats =
            "Rec. Lv." + std::to_string(info.recommendedLevel) +
            "   Enemy Lv." + std::to_string(info.enemyLevel) +
            "   Floors: " + std::to_string(static_cast<int>(info.layout.size()));
        renderText(stats, panel.x + kPad, cy, 160, 170, 200);
        cy += 24.f;
    }

    // Horizontal separator.
    fillRect({panel.x + kPad, cy, panel.w - kPad * 2.f, 1.f}, 50, 55, 75, 120);
    cy += 12.f;

    // Description — word-wrapped manually into fixed-width lines.
    if (m_font && !info.description.empty())
    {
        // Approximate wrap width: panel content width / ~9.5 px per char at 16 pt.
        const std::size_t charsPerLine =
            static_cast<std::size_t>((panel.w - kPad * 2.f) / 9.5f);
        std::string remaining = info.description;
        while (!remaining.empty() && cy + 18.f < panel.y + panel.h - 80.f)
        {
            std::string line;
            if (remaining.size() <= charsPerLine)
            {
                line = remaining;
                remaining.clear();
            }
            else
            {
                // Break at last space before charsPerLine.
                std::size_t breakPos = remaining.rfind(' ', charsPerLine);
                if (breakPos == std::string::npos || breakPos == 0)
                    breakPos = charsPerLine;
                line = remaining.substr(0, breakPos);
                remaining = remaining.substr(breakPos + (remaining[breakPos] == ' ' ? 1 : 0));
            }
            renderText(line, panel.x + kPad, cy, 145, 152, 175);
            cy += 20.f;
        }
        cy += 8.f;
    }

    // Floor layout strip.
    if (!info.layout.empty())
    {
        fillRect({panel.x + kPad, cy, panel.w - kPad * 2.f, 1.f}, 50, 55, 75, 120);
        cy += 10.f;
        if (m_font)
            renderText("FLOOR LAYOUT", panel.x + kPad, cy, 100, 110, 140);
        cy += 18.f;
        drawFloorLayoutStrip(panel.x + kPad, cy, panel.w - kPad * 2.f, info.layout);
    }
}

void SDL3Renderer::drawFloorLayoutStrip(float startX, float y, float maxWidth,
                                        const std::vector<std::string> &layout)
{
    constexpr float kNodeW = 36.f;
    constexpr float kNodeH = 36.f;
    constexpr float kNodeGap = 6.f;
    constexpr float kArrowW = 10.f; // connector arrow width

    // Total width: nodes + arrows between them.
    const float totalW = static_cast<float>(layout.size()) * kNodeW + static_cast<float>(layout.size() > 1 ? layout.size() - 1 : 0) * kArrowW + static_cast<float>(layout.size() > 1 ? layout.size() - 1 : 0) * kNodeGap * 2.f;

    // If too wide, scale everything down.
    float scale = 1.f;
    if (totalW > maxWidth)
        scale = maxWidth / totalW;

    const float nW = kNodeW * scale;
    const float nH = kNodeH * scale;
    const float aW = kArrowW * scale;
    const float gap = kNodeGap * scale;

    float cx = startX;

    for (std::size_t i = 0; i < layout.size(); ++i)
    {
        const std::string &nodeType = layout[i];

        // Node color and label by type.
        Uint8 r{100}, g{100}, b{120};
        std::string label = "?";
        if (nodeType == "battle")
        {
            r = 180;
            g = 55;
            b = 55;
            label = "B";
        }
        else if (nodeType == "elite")
        {
            r = 220;
            g = 120;
            b = 30;
            label = "E";
        }
        else if (nodeType == "boss")
        {
            r = 200;
            g = 40;
            b = 40;
            label = "!";
        }
        else if (nodeType == "rest")
        {
            r = 55;
            g = 175;
            b = 80;
            label = "R";
        }
        else if (nodeType == "treasure")
        {
            r = 200;
            g = 165;
            b = 35;
            label = "T";
        }
        else if (nodeType == "shop")
        {
            r = 70;
            g = 140;
            b = 210;
            label = "S";
        }
        else if (nodeType == "event")
        {
            r = 160;
            g = 80;
            b = 200;
            label = "?";
        }

        const SDL_FRect nodeRect{cx, y, nW, nH};
        // Dark fill, colored border, label.
        renderChamferedFillRect(nodeRect, 3.f * scale,
                                toUint8(static_cast<int>(r) / 4),
                                toUint8(static_cast<int>(g) / 4),
                                toUint8(static_cast<int>(b) / 4));
        renderChamferedBorder(nodeRect, 3.f * scale, r, g, b, 220);

        // Centered label.
        if (m_font)
            renderText(label, cx + nW * 0.32f, y + nH * 0.22f, r, g, b);

        cx += nW;

        // Arrow connector between nodes.
        if (i + 1 < layout.size())
        {
            cx += gap;
            const float arrowCy = y + nH * 0.5f;
            const SDL_FPoint z{0.f, 0.f};
            const SDL_FColor ac{0.5f, 0.55f, 0.7f, 0.70f};
            const SDL_Vertex av[3] = {
                {{cx + aW, arrowCy}, ac, z},
                {{cx, arrowCy - nH * 0.2f}, ac, z},
                {{cx, arrowCy + nH * 0.2f}, ac, z},
            };
            SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
            SDL_RenderGeometry(m_renderer, nullptr, av, 3, nullptr, 0);
            SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_NONE);
            cx += aW + gap;
        }
    }
}

void SDL3Renderer::clearBattleCache()
{
    m_cachedPlayerParty = nullptr;
    m_cachedEnemyParty = nullptr;
    m_cachedActiveCharacter = nullptr;
    m_cachedActiveParty = nullptr;
    m_cachedResonanceField = nullptr;
    m_cachedTurnOrder.clear();
    m_highlightedTargetIndex = -1;
    m_logExpanded = false;
    m_rfTriggerExpiry = 0;
    m_cachedRfGauge = 0;
    m_hpBars.clear();
    m_toughnessBars.clear();
    m_rfGaugeDisplayed = 0.f;
    m_rfGaugeTarget = 0.f;
    m_breakFlashUnit = nullptr;
    m_breakFlashExpiry = 0;
    m_deathFlashUnit = nullptr;
    m_deathFlashExpiry = 0;
    m_damageNumbers.clear();
    clearDamageTextures();
    clearTextCache();
    m_fractureLines.clear();
    m_stripSlideOffset = 0.f;
    m_rfGaugePulse = 1.0f;
    m_shakes.clear();
    m_actedThisCycle.clear();
    m_dungeonSelectInfos.clear();
    m_dungeonSelectTitle.clear();
    redrawAll();
}

void SDL3Renderer::setLogScrollOffset(int delta)
{
    m_logScrollOffset = std::max(0, std::min(m_logScrollOffset + delta,
                                             static_cast<int>(m_log.size()) - 1));
    redrawAll();
}

void SDL3Renderer::expandLog(bool expand)
{
    m_logExpanded = expand;
    redrawAll();
}

bool SDL3Renderer::isLogScrollable() const
{
    constexpr float lineH = 18.f;
    const int vis = static_cast<int>((m_logPanel.h - 4.f) / lineH);
    return static_cast<int>(m_log.size()) > vis;
}

int SDL3Renderer::getActionRowAt(int x, int y) const
{
    float logicalX{}, logicalY{};
    SDL_RenderCoordinatesFromWindow(m_renderer,
                                    static_cast<float>(x), static_cast<float>(y),
                                    &logicalX, &logicalY);
    if (logicalX < m_actionMenu.x || logicalX > m_actionMenu.x + m_actionMenu.w)
        return -1;
    constexpr float firstY = 26.f; // offset from panel top, matches drawActionMenuPanel
    constexpr float rowH = 22.f;
    const float relY = logicalY - m_actionMenu.y;
    if (relY < firstY)
        return -1;
    const int row = static_cast<int>((relY - firstY) / rowH);
    return (row >= 0 && row < 6) ? row : -1;
}

int SDL3Renderer::getUnitCardAt(int x, int y, bool isPlayerSide) const
{
    float logicalX{}, logicalY{};
    SDL_RenderCoordinatesFromWindow(m_renderer,
                                    static_cast<float>(x), static_cast<float>(y),
                                    &logicalX, &logicalY);
    const SDL_FRect &panel = isPlayerSide ? m_playerPanel : m_enemyPanel;
    if (logicalX < panel.x || logicalX > panel.x + panel.w ||
        logicalY < panel.y || logicalY > panel.y + panel.h)
        return -1;

    const Party *party = isPlayerSide ? m_cachedPlayerParty : m_cachedEnemyParty;
    if (!party)
        return -1;

    float cardY = panel.y + 8.f;
    int aliveIdx = 0;
    for (std::size_t i = 0; i < party->size(); ++i)
    {
        const Unit *u = party->getUnitAt(i);
        if (!u)
            continue;
        if (!u->isAlive() && isPlayerSide)
        {
            // Dead card height — must match playerCardHeight dead-card path.
            cardY += kCardTopPad + kPCNameH + 6.f + kPCBottomGap;
            continue;
        }
        const bool isActive = (m_cachedActiveCharacter && u == m_cachedActiveCharacter);
        const float cardBottom = cardY + (isPlayerSide ? playerCardHeight(u, isActive)
                                                       : enemyCardHeight(u));
        if (logicalY >= cardY && logicalY < cardBottom)
            return aliveIdx;
        cardY = cardBottom;
        // u is non-null here (checked above). Explicit guard silences static-analysis
        // false positives caused by the dynamic_cast inside playerCardHeight.
        if (u && u->isAlive())
            ++aliveIdx;
    }
    return -1;
}

void SDL3Renderer::renderTooltip(const std::string &name, float hpFraction,
                                 const std::string &effectSummary, int screenX, int screenY)
{
    float logicalX{}, logicalY{};
    SDL_RenderCoordinatesFromWindow(m_renderer,
                                    static_cast<float>(screenX), static_cast<float>(screenY),
                                    &logicalX, &logicalY);
    const float tx = logicalX + 12.f;
    const float ty = logicalY - 40.f;
    fillRect({tx, ty, 180.f, 38.f}, 30, 30, 50, 220);
    renderText(name + "  HP:" + std::to_string(static_cast<int>(hpFraction * 100)) + "%",
               tx + 4.f, ty + 2.f, 220, 220, 220);
    if (!effectSummary.empty())
        renderText(effectSummary, tx + 4.f, ty + 18.f, 180, 180, 200);
}

void SDL3Renderer::onWindowResized(int /*w*/, int /*h*/)
{
    redrawAll();
}