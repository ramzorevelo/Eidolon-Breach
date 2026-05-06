#pragma once
/**
 * @file SDL3InputHandler.h
 * @brief SDL3 keyboard implementation of IInputHandler.
 *        Blocks on SDL_WaitEvent until a valid key is pressed.
 *        Q/E/1/2/R/V map to action indices 0–5.
 *        Left/Right arrows map to target indices; Enter confirms.
 */

#include "UI/IInputHandler.h"
#include "UI/IRenderer.h"
#include <cstddef>

class SDL3InputHandler : public IInputHandler
{
  public:
    explicit SDL3InputHandler(IRenderer &renderer) : m_renderer{renderer} {}

    /**
     * @brief Block until the player presses a valid action key.
     *        Keys: Q=0, E=1, 1=2, 2=3, R=4, V=5.
     *        Only indices in [0, numActions) are accepted.
     * @param numActions Number of available actions.
     * @return 0-based action index.
     */
    std::size_t getActionChoice(std::size_t numActions) override;

    /**
     * @brief Block until the player selects a target.
     *        Left/Right arrows cycle; Enter confirms the highlighted index.
     * @param numTargets Number of valid targets.
     * @return 0-based target index.
     */
    std::size_t getTargetChoice(std::size_t numTargets) override;
    std::size_t getMenuChoice(std::size_t numOptions) override;

    void setMenuContext(const std::string &title,
                        const std::vector<std::string> &options)
    {
        m_menuTitle = title;
        m_menuOptions = options;
    }
  private:
    IRenderer &m_renderer;
    std::size_t m_bufferedActionIdx{std::numeric_limits<std::size_t>::max()};
    std::string m_menuTitle{};
    std::vector<std::string> m_menuOptions{};
};