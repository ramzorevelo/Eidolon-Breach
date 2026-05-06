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
    // Consume key buffered during a cancelled target selection.
    if (m_bufferedActionIdx != std::numeric_limits<std::size_t>::max())
    {
        const std::size_t idx = m_bufferedActionIdx;
        m_bufferedActionIdx = std::numeric_limits<std::size_t>::max();
        if (idx < numActions)
            return idx;
        // Buffered key out of range — fall through to normal input.
    }

    SDL_Event event{};
    while (SDL_WaitEvent(&event))
    {
        if (event.type == SDL_EVENT_QUIT)
            return numActions;
        if (event.type != SDL_EVENT_KEY_DOWN)
            continue;
        for (std::size_t i = 0; i < kNumActionKeys && i < numActions; ++i)
            if (event.key.key == kActionKeys[i])
                return i;
    }
    return 0;
}

std::size_t SDL3InputHandler::getTargetChoice(std::size_t numTargets)
{
    if (numTargets == 0)
        return 0;

    std::size_t current = 0;
    SDL_Event event{};

    while (SDL_WaitEvent(&event))
    {
        if (event.type == SDL_EVENT_QUIT)
            return 0;
        if (event.type != SDL_EVENT_KEY_DOWN)
            continue;

        // ESC — cancel targeting, return to action menu.
        if (event.key.key == SDLK_ESCAPE)
        {
            m_renderer.clearTargetHighlight();
            return IInputHandler::kCancelChoice;
        }

        // Action key pressed during targeting — cancel and buffer the key.
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
                m_renderer.updateTargetHighlight(static_cast<int>(current));
            }
            break;
        case SDLK_DOWN:
            if (current + 1 < numTargets)
            {
                ++current;
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
    return 0;
}