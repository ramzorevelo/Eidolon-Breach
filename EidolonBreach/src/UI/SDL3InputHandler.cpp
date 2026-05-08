/**
 * @file SDL3InputHandler.cpp
 * @brief SDL3InputHandler implementation.
 */

#include "UI/SDL3InputHandler.h"
#include <SDL3/SDL.h>


static const SDL_Keycode kActionKeys[] = {
    SDLK_Q, SDLK_E, SDLK_1, SDLK_2, SDLK_R, SDLK_V};
static constexpr std::size_t kNumActionKeys = 6;


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

        if (event.type == SDL_EVENT_KEY_DOWN)
        {
            if (event.key.repeat)
                continue;
            for (std::size_t i = 0; i < kNumActionKeys && i < numActions; ++i)
                if (event.key.key == kActionKeys[i])
                    return i;

            // Up/Down expand log when no targeting is active.
            if (event.key.key == SDLK_UP || event.key.key == SDLK_DOWN)
            {
                if (m_renderer.isLogScrollable())
                {
                    m_renderer.expandLog(true);
                    const int delta = (event.key.key == SDLK_UP) ? 1 : -1;
                    m_renderer.setLogScrollOffset(delta);
                }
                continue;
            }
            if (event.key.key == SDLK_ESCAPE)
            {
                m_renderer.expandLog(false);
                continue;
            }
        }

        if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
            event.button.button == SDL_BUTTON_LEFT)
        {
            const int row = m_renderer.getActionRowAt(
                static_cast<int>(event.button.x),
                static_cast<int>(event.button.y));
            if (row >= 0 && static_cast<std::size_t>(row) < numActions)
                return static_cast<std::size_t>(row);
        }

        if (event.type == SDL_EVENT_MOUSE_WHEEL)
        {
            if (m_renderer.isLogScrollable())
            {
                m_renderer.expandLog(true);
                m_renderer.setLogScrollOffset(static_cast<int>(event.wheel.y));
            }
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

        if (event.type == SDL_EVENT_KEY_DOWN)
        {
            if (event.key.repeat)
                continue;
            if (event.key.key == SDLK_ESCAPE)
            {
                m_renderer.clearTargetHighlight();
                return IInputHandler::kCancelChoice;
            }

            for (std::size_t i = 0; i < kNumActionKeys; ++i)
            {
                if (event.key.key == kActionKeys[i])
                {
                    m_bufferedActionIdx = i;
                    m_renderer.clearTargetHighlight();
                    return IInputHandler::kCancelChoice;
                }
            }

            switch (event.key.key)
            {
            case SDLK_UP:
                if (current > 0)
                {
                    --current;
                    m_pendingTargetIdx = static_cast<int>(current);
                    m_renderer.updateTargetHighlight(static_cast<int>(current));
                }
                break;
            case SDLK_DOWN:
                if (current + 1 < numTargets)
                {
                    ++current;
                    m_pendingTargetIdx = static_cast<int>(current);
                    m_renderer.updateTargetHighlight(static_cast<int>(current));
                }
                break;
            case SDLK_RETURN:
            case SDLK_KP_ENTER:
                return current;
            default:
                break;
            }
        }

        if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
        {
            if (event.button.button == SDL_BUTTON_RIGHT)
            {
                m_renderer.clearTargetHighlight();
                return IInputHandler::kCancelChoice;
            }

            if (event.button.button == SDL_BUTTON_LEFT)
            {
                const bool isPlayerSide = !m_renderer.isHighlightingEnemies();
                const int card = m_renderer.getUnitCardAt(
                    static_cast<int>(event.button.x),
                    static_cast<int>(event.button.y),
                    isPlayerSide);
                if (card >= 0 && static_cast<std::size_t>(card) < numTargets)
                {
                    if (m_pendingTargetIdx == card)
                    {
                        // Second click on same card confirms.
                        return static_cast<std::size_t>(card);
                    }
                    // First click: highlight but do not confirm.
                    m_pendingTargetIdx = card;
                    current = static_cast<std::size_t>(card);
                    m_renderer.updateTargetHighlight(card);
                }
            }
        }

        if (event.type == SDL_EVENT_MOUSE_MOTION)
        {
            const bool isPlayerSide = !m_renderer.isHighlightingEnemies();
            const int card = m_renderer.getUnitCardAt(
                static_cast<int>(event.motion.x),
                static_cast<int>(event.motion.y),
                isPlayerSide);
            if (card >= 0 && static_cast<std::size_t>(card) < numTargets)
            {
                // Hover tooltip handled by renderer.
            }
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

        if (event.type == SDL_EVENT_KEY_DOWN)
        {
            if (event.key.repeat)
                continue;
            if (event.key.key == SDLK_ESCAPE)
                return IInputHandler::kCancelChoice;
            switch (event.key.key)
            {
            case SDLK_UP:
                if (current > 0)
                {
                    --current;
                    m_renderer.renderSelectionMenu(m_menuTitle, m_menuOptions, current);
                }
                break;
            case SDLK_DOWN:
                if (current + 1 < numOptions)
                {
                    ++current;
                    m_renderer.renderSelectionMenu(m_menuTitle, m_menuOptions, current);
                }
                break;
            case SDLK_RETURN:
            case SDLK_KP_ENTER:
                return current;
            default:
                for (std::size_t i = 0; i < numOptions && i < 9; ++i)
                {
                    if (event.key.key == static_cast<SDL_Keycode>(SDLK_1 + static_cast<int>(i)))
                        return i;
                }
                break;
            }
        }

        if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
            event.button.button == SDL_BUTTON_LEFT)
        {
            // Menu rows rendered starting at ~18% window height + 36px offset.
            const float firstRowY = static_cast<float>(m_renderer.getWindowHeight()) * 0.18f + 36.f;
            const float rowH = 30.f;
            const float fy = static_cast<float>(event.button.y);

            if (fy >= firstRowY)
            {
                const int row = static_cast<int>((fy - firstRowY) / rowH);
                if (row >= 0 && static_cast<std::size_t>(row) < numOptions)
                    return static_cast<std::size_t>(row);
            }
        }

        if (event.type == SDL_EVENT_MOUSE_WHEEL)
        {
            m_renderer.setLogScrollOffset(static_cast<int>(event.wheel.y));
        }
    }
    return 0;
}