/**
 * @file SDL3InputHandler.cpp
 * @brief SDL3InputHandler implementation.
 */

#include "UI/SDL3InputHandler.h"
#include <SDL3/SDL.h>


static const SDL_Keycode kActionKeys[] = {
    SDLK_Q, SDLK_E, SDLK_1, SDLK_2, SDLK_R, SDLK_V};
static constexpr std::size_t kNumActionKeys = 6;

void SDL3InputHandler::handleLogScroll(SDL_Keycode key)
{
    if (!m_layout.isLogScrollable())
        return;
    m_layout.expandLog(true);
    m_layout.setLogScrollOffset(key == SDLK_UP ? 1 : -1);
}

std::optional<std::size_t> SDL3InputHandler::handleTargetKey(
    SDL_Keycode key, std::size_t &current, std::size_t numTargets)
{
    if (key == SDLK_ESCAPE)
    {
        m_renderer.clearTargetHighlight();
        return IInputHandler::kCancelChoice;
    }
    for (std::size_t i = 0; i < kNumActionKeys; ++i)
    {
        if (key == kActionKeys[i])
        {
            m_bufferedActionIdx = i;
            m_renderer.clearTargetHighlight();
            return IInputHandler::kCancelChoice;
        }
    }
    if (key == SDLK_UP && current > 0)
    {
        --current;
        m_pendingTargetIdx = static_cast<int>(current);
        m_renderer.updateTargetHighlight(static_cast<int>(current));
    }
    else if (key == SDLK_DOWN && current + 1 < numTargets)
    {
        ++current;
        m_pendingTargetIdx = static_cast<int>(current);
        m_renderer.updateTargetHighlight(static_cast<int>(current));
    }
    else if (key == SDLK_RETURN || key == SDLK_KP_ENTER)
        return current;
    return std::nullopt;
}

std::optional<std::size_t> SDL3InputHandler::handleTargetMouse(
    const SDL_Event &e, std::size_t &current, std::size_t numTargets)
{
    if (e.button.button == SDL_BUTTON_RIGHT)
    {
        m_renderer.clearTargetHighlight();
        return IInputHandler::kCancelChoice;
    }
    if (e.button.button == SDL_BUTTON_LEFT)
    {
        const bool isPlayerSide = !m_layout.isHighlightingEnemies();
        const int card = m_layout.getUnitCardAt(
            static_cast<int>(e.button.x), static_cast<int>(e.button.y), isPlayerSide);
        if (card >= 0 && static_cast<std::size_t>(card) < numTargets)
        {
            if (m_pendingTargetIdx == card)
                return static_cast<std::size_t>(card);
            m_pendingTargetIdx = card;
            current = static_cast<std::size_t>(card);
            m_renderer.updateTargetHighlight(card);
        }
    }
    return std::nullopt;
}

std::optional<std::size_t> SDL3InputHandler::handleMenuKey(
    SDL_Keycode key, std::size_t &current, std::size_t numOptions)
{
    if (key == SDLK_ESCAPE)
        return IInputHandler::kCancelChoice;
    if (key == SDLK_UP && current > 0)
    {
        --current;
        m_renderer.renderSelectionMenu(m_menuTitle, m_menuOptions, current);
    }
    else if (key == SDLK_DOWN && current + 1 < numOptions)
    {
        ++current;
        m_renderer.renderSelectionMenu(m_menuTitle, m_menuOptions, current);
    }
    else if (key == SDLK_RETURN || key == SDLK_KP_ENTER)
        return current;
    else
    {
        for (std::size_t i = 0; i < numOptions && i < 9; ++i)
            if (key == static_cast<SDL_Keycode>(SDLK_1 + static_cast<int>(i)))
                return i;
    }
    return std::nullopt;
}
std::size_t SDL3InputHandler::getActionChoice(std::size_t numActions)
{
    if (m_bufferedActionIdx != std::numeric_limits<std::size_t>::max())
    {
        const std::size_t idx = m_bufferedActionIdx;
        m_bufferedActionIdx = std::numeric_limits<std::size_t>::max();
        if (idx < numActions)
            return idx;
    }

    SDL_Event event{};
    while (SDL_WaitEvent(&event))
    {
        if (event.type == SDL_EVENT_QUIT)
            throw QuitException{};

        if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat)
        {
            for (std::size_t i = 0; i < kNumActionKeys && i < numActions; ++i)
                if (event.key.key == kActionKeys[i])
                    return i;
            if (event.key.key == SDLK_UP || event.key.key == SDLK_DOWN)
                handleLogScroll(event.key.key);
            else if (event.key.key == SDLK_ESCAPE)
                m_layout.expandLog(false);
        }
        if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_LEFT)
        {
            const int row = m_layout.getActionRowAt(
                static_cast<int>(event.button.x), static_cast<int>(event.button.y));
            if (row >= 0 && static_cast<std::size_t>(row) < numActions)
                return static_cast<std::size_t>(row);
        }
        if (event.type == SDL_EVENT_MOUSE_WHEEL && m_layout.isLogScrollable())
        {
            m_layout.expandLog(true);
            m_layout.setLogScrollOffset(static_cast<int>(event.wheel.y));
        }
    }
    return 0;
}

std::size_t SDL3InputHandler::getTargetChoice(std::size_t numTargets)
{
    if (numTargets == 0)
        return 0;

    std::size_t current = 0;
    m_pendingTargetIdx = -1;
    SDL_Event event{};

    while (SDL_WaitEvent(&event))
    {
        if (event.type == SDL_EVENT_QUIT)
            throw QuitException{};

        if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat)
        {
            if (const auto r = handleTargetKey(event.key.key, current, numTargets))
                return *r;
        }
        if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
        {
            if (const auto r = handleTargetMouse(event, current, numTargets))
                return *r;
        }
    }
    return 0;
}

std::size_t SDL3InputHandler::getMenuChoice(std::size_t numOptions)
{
    if (numOptions == 0)
        return 0;

    std::size_t current = 0;
    SDL_Event event{};

    while (SDL_WaitEvent(&event))
    {
        if (event.type == SDL_EVENT_QUIT)
            throw QuitException{};

        if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat)
        {
            if (const auto r = handleMenuKey(event.key.key, current, numOptions))
                return *r;
        }
        if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_LEFT)
        {
            const float firstRowY = static_cast<float>(m_layout.getWindowHeight()) * 0.18f + 36.f;
            const float fy = static_cast<float>(event.button.y);
            if (fy >= firstRowY)
            {
                const int row = static_cast<int>((fy - firstRowY) / 30.f);
                if (row >= 0 && static_cast<std::size_t>(row) < numOptions)
                    return static_cast<std::size_t>(row);
            }
        }
        if (event.type == SDL_EVENT_MOUSE_WHEEL)
            m_layout.setLogScrollOffset(static_cast<int>(event.wheel.y));
    }
    return 0;
}