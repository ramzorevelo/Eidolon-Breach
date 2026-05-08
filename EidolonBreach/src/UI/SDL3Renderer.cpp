/**
 * @file SDL3Renderer.cpp
 * @brief SDL3Renderer implementation.
 *
 * Rendering contract: every public render*() method updates cached state and
 * calls redrawAll(). redrawAll() is the single point that clears, composites
 * all panels, and presents. No other function calls SDL_RenderPresent.
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
#include <stdexcept>
#include <algorithm>
#include <string>

// Internal helpers 

namespace
{

/// Truncate a string to maxChars, appending '~' if cut.
std::string truncate(const std::string &s, std::size_t maxChars)
{
    if (s.size() <= maxChars)
        return s;
    return s.substr(0, maxChars - 1) + "~";
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

// Construction / destruction 

SDL3Renderer::SDL3Renderer(const char *windowTitle, int width, int height)
    : m_windowWidth{width}, m_windowHeight{height}
{
    if (!SDL_Init(SDL_INIT_VIDEO))
        throw std::runtime_error{
            std::string{"SDL3Renderer: SDL_Init failed: "} + SDL_GetError()};

    if (!TTF_Init())
        throw std::runtime_error{
            std::string{"SDL3Renderer: TTF_Init failed: "} + SDL_GetError()};

    m_window = SDL_CreateWindow(windowTitle, width, height, SDL_WINDOW_RESIZABLE);
    if (!m_window)
        throw std::runtime_error{
            std::string{"SDL3Renderer: CreateWindow failed: "} + SDL_GetError()};

    m_renderer = SDL_CreateRenderer(m_window, nullptr);
    if (!m_renderer)
        throw std::runtime_error{
            std::string{"SDL3Renderer: CreateRenderer failed: "} + SDL_GetError()};

    computePanelLayout(width, height);

    m_font = TTF_OpenFont("data/fonts/ShareTechMono-Regular.ttf", 16);
    if (!m_font)
        SDL_Log("SDL3Renderer: font load failed: %s", SDL_GetError());

    m_fontLarge = TTF_OpenFont("data/fonts/ShareTechMono-Regular.ttf", 22);
    if (!m_fontLarge)
        SDL_Log("SDL3Renderer: large font load failed: %s", SDL_GetError());


    // Initial clear so the window isn't garbage before the first render call.
    SDL_SetRenderDrawColor(m_renderer, 18, 18, 24, 255);
    SDL_RenderClear(m_renderer);
    SDL_RenderPresent(m_renderer);
}

SDL3Renderer::~SDL3Renderer()
{
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

// Layout 

void SDL3Renderer::computePanelLayout(int w, int h)
{
    const float fw = static_cast<float>(w);
    const float fh = static_cast<float>(h);
    const float turnH = fh * 0.10f;
    const float logH = fh * 0.18f;
    const float hintH = 26.f;
    const float mainH = fh - turnH - logH - hintH;
    const float menuW = fw * 0.42f;
    const float logW = fw - menuW;
    const float statW = fw * 0.20f; 
    const float rfW = fw * 0.08f;

    m_turnOrderPanel = {0.f, 0.f, fw, turnH};
    m_playerPanel = {0.f, turnH, statW, mainH};
    m_centerPanel = {(fw - rfW) * 0.5f, turnH, rfW, mainH};
    m_enemyPanel = {fw - statW, turnH, statW, mainH};
    m_actionMenu = {0.f, turnH + mainH, menuW, logH};
    m_logPanel = {menuW, turnH + mainH, logW, logH};
    m_hintBar = {0.f, fh - hintH, fw, hintH};
}

// Core render loop 

void SDL3Renderer::redrawAll()
{
    // Clear the entire backbuffer.
    SDL_SetRenderDrawColor(m_renderer, 18, 18, 24, 255);
    SDL_RenderClear(m_renderer);

    // Draw every panel from cached state.
    drawTurnOrderStrip();

    if (m_cachedPlayerParty || m_cachedEnemyParty)
    {
        drawPlayerPanel();
        drawEnemyPanel();
    }

    if (m_cachedResonanceField)
        drawCenterPanel();

    if (m_cachedActiveCharacter && m_cachedActiveParty)
        drawActionMenuPanel();

    drawLogPanel();
    drawHintBarPanel();
    // RF trigger flash overlay.
    if (SDL_GetTicks() < m_rfTriggerExpiry)
    {
        const SDL_Color tc{affinityColor(m_rfTriggerAffinity)};
        fillRect({0.f, 0.f, static_cast<float>(m_windowWidth), static_cast<float>(m_windowHeight)},
                 tc.r, tc.g, tc.b, 50);
        renderTextEx(m_fontLarge, "RESONANCE TRIGGER",
                     static_cast<float>(m_windowWidth) * 0.3f,
                     static_cast<float>(m_windowHeight) * 0.45f,
                     tc.r, tc.g, tc.b);
    }

    SDL_RenderPresent(m_renderer);
}

// Per-panel draw helpers 

void SDL3Renderer::drawTurnOrderStrip()
{
    fillRect(m_turnOrderPanel, 30, 30, 45, 255);

    if (m_cachedTurnOrder.empty() || !m_font)
        return;

    const float slotW = m_turnOrderPanel.w / static_cast<float>(m_cachedTurnOrder.size());
    const float badgeH = m_turnOrderPanel.h * 0.60f;
    const float badgeY = m_turnOrderPanel.y + (m_turnOrderPanel.h - badgeH) * 0.5f;

    for (std::size_t i = 0; i < m_cachedTurnOrder.size(); ++i)
    {
        const TurnSlot &slot = m_cachedTurnOrder[i];
        if (!slot.unit)
            continue;

        const float badgeX = m_turnOrderPanel.x + static_cast<float>(i) * slotW + slotW * 0.05f;
        const float badgeW = slotW * 0.90f;
        const SDL_FRect badge{badgeX, badgeY, badgeW, badgeH};
        const SDL_Color ac{affinityColor(slot.unit->getAffinity())};

        // Player units: full color. Enemies: dimmed to 40%.
        if (slot.isPlayer)
            fillRect(badge, ac.r, ac.g, ac.b);
        else
            fillRect(badge, ac.r * 2 / 5, ac.g * 2 / 5, ac.b * 2 / 5);

        // Name initial(s) centered in badge.
        const std::string label = slot.unit->getName().substr(0, 1);
        renderText(label,
                   badgeX + badgeW * 0.30f,
                   badgeY + badgeH * 0.15f,
                   255, 255, 255);
    }
}

void SDL3Renderer::drawPlayerPanel()
{
    fillRect(m_playerPanel, 22, 22, 32, 255);

    if (!m_cachedPlayerParty || !m_font)
        return;

    const float barW = m_playerPanel.w - 12.f;
    const float barX = m_playerPanel.x + 6.f;
    const float nameH = 18.f;
    const float barH = 6.f;
    const float thinH = 4.f;
    const float gapH = 4.f;

    float py = m_playerPanel.y + 8.f;
    int aliveIdx = 0;

    for (std::size_t i = 0; i < m_cachedPlayerParty->size(); ++i)
    {
        const Unit *u = m_cachedPlayerParty->getUnitAt(i);
        if (!u)
            continue;

        const auto *pc = dynamic_cast<const PlayableCharacter *>(u);
        const bool highlighted = (!m_highlightingEnemies && m_highlightedTargetIndex == aliveIdx);
        const bool isActive = (m_cachedActiveCharacter && u == m_cachedActiveCharacter);
        const SDL_Color ac{affinityColor(u->getAffinity())};

        if (highlighted)
            fillRect({m_playerPanel.x, py, m_playerPanel.w, nameH}, 60, 80, 60, 255);

        if (isActive)
        {
            // Left-edge accent strip for the active card.
            fillRect({m_playerPanel.x, py, 3.f, nameH}, ac.r, ac.g, ac.b);
        }

        if (u->isAlive())
        {
            const std::string nameStr = (highlighted ? "> " : "  ") + u->getName();
            renderText(nameStr, barX, py, ac.r, ac.g, ac.b);
            ++aliveIdx;
        }
        else
        {
            renderText("  " + u->getName() + " [KO]", barX, py, 80, 80, 80);
            py += nameH + 10.f;
            continue;
        }
        py += nameH;

        // HP bar (always shown).
        const float hpFrac = (u->getMaxHp() > 0)
                                 ? static_cast<float>(u->getHp()) / static_cast<float>(u->getMaxHp())
                                 : 0.f;
        renderBar({barX, py, barW, barH}, hpFrac, 60, 190, 60, 70, 25, 25);
        py += barH + gapH;

        if (pc && isActive)
        {
            // Full detail: Energy bar.
            const float enFrac = static_cast<float>(pc->getEnergy()) / static_cast<float>(PlayableCharacter::kMaxEnergy);
            renderBar({barX, py, barW, thinH}, enFrac, 220, 180, 0, 45, 35, 0);
            py += thinH + gapH;

            // Full detail: Exposure bar.
            const float expFrac = static_cast<float>(pc->getExposure()) / static_cast<float>(PlayableCharacter::kMaxExposure);
            renderBar({barX, py, barW, thinH}, expFrac, 220, 75, 0, 35, 18, 0);
            py += thinH;

            if (pc->isFractured())
            {
                renderText("[FRAC]", barX, py, 200, 40, 40);
                py += 14.f;
            }
        }
        else if (pc)
        {
            // Compact: Exposure bar only.
            const float expFrac = static_cast<float>(pc->getExposure()) / static_cast<float>(PlayableCharacter::kMaxExposure);
            renderBar({barX, py, barW, thinH}, expFrac, 220, 75, 0, 35, 18, 0);
            py += thinH;
        }
        py += 10.f;
    }
}

void SDL3Renderer::drawEnemyPanel()
{
    fillRect(m_enemyPanel, 22, 22, 32, 255);

    if (!m_cachedEnemyParty || !m_font)
        return;

    const float nameH = 18.f;
    const float barH = 6.f;
    const float thinH = 4.f;
    const float gapH = 4.f;
    const float barW = m_enemyPanel.w - 12.f;
    const float barX = m_enemyPanel.x + 6.f;

    float ey = m_enemyPanel.y + 8.f;

    int aliveIdx = 0;
    for (std::size_t i = 0; i < m_cachedEnemyParty->size(); ++i)
    {
        const Unit *u = m_cachedEnemyParty->getUnitAt(i);
        if (!u || !u->isAlive())
            continue;

        const bool highlighted = (m_highlightingEnemies && m_highlightedTargetIndex == aliveIdx);
        const SDL_Color ac{affinityColor(u->getAffinity())};

        // Draw highlight bar behind name
        if (highlighted)
            fillRect({m_enemyPanel.x, ey, m_enemyPanel.w, nameH}, 60, 60, 80, 255);

        const std::string nameStr = (highlighted ? "> " : "  ") + u->getName();
        renderText(nameStr, barX, ey, ac.r, ac.g, ac.b);
        ey += nameH;

        // HP bar: red fill.
        const float hpFrac = (u->getMaxHp() > 0)
                                 ? static_cast<float>(u->getHp()) / static_cast<float>(u->getMaxHp())
                                 : 0.f;
        renderBar({barX, ey, barW, barH}, hpFrac, 200, 55, 55, 70, 25, 25);
        ey += barH + gapH;

        // Toughness bar: white fill (Enemy only).
        const auto *e = dynamic_cast<const Enemy *>(u);
        if (e)
        {
            const float toughFrac = (e->getMaxToughness() > 0)
                                        ? static_cast<float>(e->getToughness()) / static_cast<float>(e->getMaxToughness())
                                        : 0.f;
            const bool broken = e->isBroken();
            renderBar({barX, ey, barW, thinH},
                      toughFrac,
                      broken ? 255 : 210,
                      broken ? 80 : 210,
                      broken ? 0 : 210,
                      50, 50, 50);
        }
        ey += thinH + 10.f;
        ++aliveIdx;
    }
}

void SDL3Renderer::drawCenterPanel()
{
    fillRect(m_centerPanel, 20, 20, 32, 255);

    if (!m_cachedResonanceField || !m_font)
        return;

    renderText("RF", m_centerPanel.x + 4.f, m_centerPanel.y + 4.f, 160, 160, 220);

    const std::array<Affinity, 5> affinities{
        Affinity::Blaze, Affinity::Frost, Affinity::Tempest,
        Affinity::Terra, Affinity::Aether};

    float barY = m_centerPanel.y + 22.f;
    const float barW = m_centerPanel.w - 8.f;

    for (Affinity a : affinities)
    {
        const float votes = static_cast<float>(m_cachedResonanceField->getVotes(a));
        const float maxVotes = 5.f;
        const SDL_Color ac{affinityColor(a)};
        renderBar({m_centerPanel.x + 4.f, barY, barW, 5.f},
                  votes / maxVotes, ac.r, ac.g, ac.b, 28, 28, 38);
        barY += 9.f;
    }
}

void SDL3Renderer::drawActionMenuPanel()
{
    fillRect(m_actionMenu, 20, 20, 30, 255);

    if (!m_cachedActiveCharacter || !m_cachedActiveParty || !m_font)
        return;

    // Header: character name + shared SP.
    const std::string header = m_cachedActiveCharacter->getName() + "  SP:" + std::to_string(m_cachedActiveParty->getSp());
    renderText(header, m_actionMenu.x + 6.f, m_actionMenu.y + 4.f, 220, 220, 220);

    static const std::array<std::string, 6> kKeys{"Q", "E", "1", "2", "R", "V"};

    // Max label chars that fit the panel at 16px monospace (~9.6px/char → ~31 chars).
    static constexpr std::size_t kMaxLabel = 30;

    float rowY = m_actionMenu.y + 22.f;
    const float rowH = 20.f;
    std::size_t keyIdx = 0;

    for (const auto &action : m_cachedActiveCharacter->getAbilities())
    {
        if (!action || keyIdx >= kKeys.size())
            break;

        // Mirror selectActionIndex() filtering: skip hidden Arch Skill.
        const ActionData &data = action->getActionData();
        if (data.category == ActionCategory::ArchSkill && !m_cachedActiveCharacter->isArchSkillUnlocked())
            continue;

        const bool available = action->isAvailable(*m_cachedActiveCharacter,
                                                   *m_cachedActiveParty);
        const std::string row = "[" + kKeys[keyIdx] + "] " + action->label();

        if (available)
        {
            const SDL_Color ac{affinityColor(m_cachedActiveCharacter->getAffinity())};
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
    fillRect(m_logPanel, 14, 14, 20, 255);

    if (!m_font || m_log.empty())
        return;

    const float lineH = 18.f;
    const int visibleLines = static_cast<int>(m_logPanel.h / lineH);
    const int total = static_cast<int>(m_log.size());
    const int start = std::max(0, total - visibleLines - m_logScrollOffset);

    float ly = m_logPanel.y + 4.f;

    for (int i = start; i < total && ly + lineH <= m_logPanel.y + m_logPanel.h; ++i)
    {
        // Distance from the newest entry: 0 = newest, grows toward older.
        const int age = total - 1 - i;
        // Newest line: 210. Each step older loses 25, floor at 70.
        const Uint8 c = static_cast<Uint8>(std::max(70, 210 - age * 25));
        renderText(m_log[static_cast<std::size_t>(i)],
                   m_logPanel.x + 6.f, ly, c, c, c);
        ly += lineH;
    }
}

void SDL3Renderer::drawHintBarPanel()
{
    fillRect(m_hintBar, 12, 12, 18, 255);

    if (!m_font || m_cachedHint.empty())
        return;

    renderText(m_cachedHint, m_hintBar.x + 8.f, m_hintBar.y + 4.f, 120, 120, 140);
}

// Low-level drawing primitives 

void SDL3Renderer::fillRect(const SDL_FRect &rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    SDL_SetRenderDrawColor(m_renderer, r, g, b, a);
    SDL_RenderFillRect(m_renderer, &rect);
}

void SDL3Renderer::renderBar(const SDL_FRect &rect, float fraction,
                             Uint8 fr, Uint8 fg, Uint8 fb,
                             Uint8 br, Uint8 bg, Uint8 bb)
{
    fillRect(rect, br, bg, bb);

    const float clamped = fraction < 0.f ? 0.f : fraction > 1.f ? 1.f
                                                                : fraction;
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

// Each render*() method updates cached state, then calls redrawAll().

void SDL3Renderer::renderTurnOrder(const std::vector<TurnSlot> &order)
{
    m_cachedTurnOrder = order;
    redrawAll();
}

void SDL3Renderer::renderPartyStatus(const Party &playerParty, const Party &enemyParty)
{
    m_cachedPlayerParty = &playerParty;
    m_cachedEnemyParty = &enemyParty;
    redrawAll();
}

void SDL3Renderer::renderResonanceField(const ResonanceField &field)
{
    const int newGauge = field.getGauge();

    // Detect RF trigger: gauge was >= 100 and just reset to 0.
    if (m_cachedRfGauge >= 100 && newGauge == 0)
    {
        m_rfTriggerAffinity = field.getLeadingAffinity();
        m_rfTriggerExpiry = SDL_GetTicks() + 700;
    }
    m_cachedRfGauge = newGauge;

    m_cachedResonanceField = &field;
    redrawAll();
}
void SDL3Renderer::renderActionMenu(const PlayableCharacter &character, const Party &party)
{
    m_cachedActiveCharacter = &character;
    m_cachedActiveParty = &party;
    redrawAll();
}

void SDL3Renderer::renderHintBar(const std::string &hint)
{
    m_cachedHint = hint;
    redrawAll();
}

void SDL3Renderer::renderMessage(const std::string &message)
{
    m_log.push_back(message);
    if (static_cast<int>(m_log.size()) > kMaxLogLines)
        m_log.erase(m_log.begin());
    m_logScrollOffset = 0;
    m_logExpanded = false;
    m_lastLogTime = SDL_GetTicks();
    redrawAll();
}

void SDL3Renderer::renderActionResult(const std::string &actorName,
                                      const ActionResult &result)
{
    std::string line = actorName + ": ";
    switch (result.type)
    {
    case ActionResult::Type::Damage:
        line += "dealt " + std::to_string(result.value) + " dmg";
        if (!result.targetName.empty())
            line += " to " + result.targetName;
        break;
    case ActionResult::Type::Heal:
        line += "healed " + std::to_string(result.value) + " HP";
        if (!result.targetName.empty())
            line += " on " + result.targetName;
        break;
    case ActionResult::Type::Skip:
        line += result.flavorText.empty() ? "skipped" : result.flavorText;
        break;
    case ActionResult::Type::Charge:
        line += "charges energy";
        break;
    }
    if (!result.flavorText.empty() && result.type != ActionResult::Type::Skip)
        line += " — " + result.flavorText;
    renderMessage(line);
}

void SDL3Renderer::renderBreak(const std::string &enemyName)
{
    renderMessage("** " + enemyName + " BROKEN **");
}

void SDL3Renderer::renderStunned(const std::string &enemyName)
{
    renderMessage(enemyName + " stunned - turn skipped");
}

void SDL3Renderer::renderVictory(const std::string &enemyName, std::optional<Drop> drop)
{
    renderMessage(enemyName + " defeated!");
    if (drop)
        renderMessage("  Drop: " + drop->itemId);
}

void SDL3Renderer::renderDefeat(const std::string &playerName)
{
    renderMessage(playerName + " has fallen.");
}

void SDL3Renderer::renderTargetList(const std::vector<std::string> & /*names*/,
                                    bool isAllyTarget)
{
    m_highlightedTargetIndex = 0;
    m_highlightingEnemies = !isAllyTarget;
    redrawAll();
}

void SDL3Renderer::clearTargetHighlight()
{
    m_highlightedTargetIndex = -1;
    redrawAll();
}

void SDL3Renderer::updateTargetHighlight(int index)
{
    m_highlightedTargetIndex = index;
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
    SDL_SetRenderDrawColor(m_renderer, 18, 18, 24, 255);
    SDL_RenderClear(m_renderer);

    if (!m_font)
    {
        SDL_RenderPresent(m_renderer);
        return;
    }

    const float cx = m_windowWidth * 0.28f;
    const float ty = m_windowHeight * 0.18f;

    // Title
    renderTextEx(m_fontLarge, "=== " + title + " ===", cx, ty, 220, 200, 100);

    const float lineH = 30.f;
    float optionY = ty + 36.f;

    for (std::size_t i = 0; i < options.size(); ++i)
    {
        const bool isSelected = (i == selected);

        if (isSelected)
        {
            const SDL_FRect highlight{cx - 4.f, optionY - 2.f,
                                      m_windowWidth * 0.50f, lineH};
            fillRect(highlight, 40, 40, 60, 255);
        }
        const std::size_t maxChars = static_cast<std::size_t>(m_windowWidth * 0.70f / 9.6f);
        std::string display = options[i];
        if (display.size() > maxChars)
            display = display.substr(0, maxChars - 1) + "~";
        const std::string row = (isSelected ? "> " : "  ") + std::to_string(i + 1) + ". " + options[i];
        const Uint8 brightness = isSelected ? 255 : 160;
        renderTextEx(m_fontLarge, row, cx, optionY,
                     brightness, brightness, isSelected ? 100 : brightness);

        optionY += lineH;
    }

    // Hint
    renderTextEx(m_fontLarge,
                 "[Up/Down] Navigate   [Enter] Confirm   [1-9] Direct select",
                 cx, m_windowHeight * 0.88f, 100, 100, 120);

    SDL_RenderPresent(m_renderer);
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
    redrawAll();
}

void SDL3Renderer::setLogScrollOffset(int delta)
{
    m_logScrollOffset = std::max(0,
                                 std::min(m_logScrollOffset + delta,
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
    const int visibleLines = static_cast<int>(m_logPanel.h / 18.f);
    return static_cast<int>(m_log.size()) > visibleLines;
}


void SDL3Renderer::renderTextEx(TTF_Font *font, const std::string &text,
                                float x, float y, Uint8 r, Uint8 g, Uint8 b)
{
    if (!font || text.empty())
        return;
    const SDL_Color color{r, g, b, 255};
    SDL_Surface *surface = TTF_RenderText_Blended(font, text.c_str(), 0, color);
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